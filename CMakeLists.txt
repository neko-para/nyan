cmake_minimum_required(VERSION 3.30)

project(nyan CXX ASM)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# user program

add_library(user STATIC src/user/lib/start.s src/user/lib/syscall.cpp)

add_executable(prog src/user/entry/test.cpp)
target_link_libraries(prog PRIVATE user)
target_compile_options(prog PRIVATE -Wall -Wextra)

add_custom_command(
  OUTPUT ${CMAKE_BINARY_DIR}/prog.o
  COMMAND ${CMAKE_COMMAND} ARGS -E copy $<TARGET_FILE:prog>
          ${CMAKE_BINARY_DIR}/prog_bin
  COMMAND
    ${CMAKE_OBJCOPY} ARGS -I binary -O elf32-i386 -B i386 --rename-section
    .data=.rodata,alloc,load,readonly,data,contents prog_bin
    ${CMAKE_BINARY_DIR}/prog.o
  DEPENDS prog
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

add_custom_target(prog_embed DEPENDS ${CMAKE_BINARY_DIR}/prog.o)

# kernel

add_executable(
  kernel
  src/main.cpp
  src/boot.s
  src/allocator/load.cpp
  src/allocator/slab.cpp
  src/allocator/utils.cpp
  src/gdt/load.cpp
  src/interrupt/impl.cpp
  src/interrupt/isr.cpp
  src/interrupt/load.cpp
  src/interrupt/syscall.s
  src/keyboard/load.cpp
  src/keyboard/message.cpp
  src/paging/kernel.cpp
  src/syscall/exit.cpp
  src/syscall/write.cpp
  src/syscall/getpid.cpp
  src/syscall/nanosleep.cpp
  src/task/switch.s
  src/task/pid.cpp
  src/task/task.cpp
  src/timer/load.cpp
  src/vga/print.cpp
  src/lib/libcxx.cpp
  src/lib/libc.cpp)
target_link_options(kernel PRIVATE -T ${CMAKE_CURRENT_LIST_DIR}/src/linker.ld)

target_compile_options(kernel PRIVATE -g -Wall -Wextra)

target_compile_options(
  kernel PRIVATE -flto=thin -funified-lto -Og -fomit-frame-pointer
                 -ffunction-sections -fdata-sections)

target_link_options(kernel PRIVATE --lto=thin --gc-sections)

# 不能开启向量化, 会产生内存引用, 提前启用sse也不行
set_source_files_properties(src/paging/kernel.cpp PROPERTIES COMPILE_FLAGS
                                                             "-mno-sse")
set_source_files_properties(src/interrupt/isr.cpp
                            PROPERTIES COMPILE_FLAGS "-mgeneral-regs-only")

set_target_properties(kernel PROPERTIES LINK_DEPENDS
                                        ${CMAKE_CURRENT_LIST_DIR}/src/linker.ld)

add_dependencies(kernel prog_embed)

target_link_directories(kernel PRIVATE sysroot/usr/lib/generic
                        sysroot/usr/lib/i686-none-elf)
target_link_libraries(kernel PRIVATE c clang_rt.builtins-i386
                                     ${CMAKE_BINARY_DIR}/prog.o)
