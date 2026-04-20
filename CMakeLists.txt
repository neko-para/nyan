cmake_minimum_required(VERSION 3.30)

project(nyan CXX ASM)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(NYAN_ENABLE_OPTIMIZE CACHE OFF)

# user program

add_library(kernel_header INTERFACE)
target_include_directories(kernel_header INTERFACE SYSTEM src/include)

add_library(
  user STATIC
  src/user/lib/src/start.s
  src/user/lib/src/errno.cpp
  src/user/lib/src/signal.cpp
  src/user/lib/src/stdio.cpp
  src/user/lib/src/stdlib.cpp
  src/user/lib/src/string.cpp
  src/user/lib/src/syscall.cpp
  src/user/lib/src/unistd.cpp)
target_include_directories(user PUBLIC src/user/lib/include)
target_link_libraries(user PUBLIC kernel_header)

set(USER_PROGRAM_OBJECTS "")
set(USER_PROGRAM_TARGETS "")

function(add_user_program NAME SOURCE)
  add_executable(${NAME} ${SOURCE})
  target_link_libraries(${NAME} PRIVATE user)
  target_compile_options(${NAME} PRIVATE -Wall -Wextra -g)

  set(BIN_FILE ${CMAKE_BINARY_DIR}/${NAME}_bin)
  set(OBJ_FILE ${CMAKE_BINARY_DIR}/${NAME}.o)

  add_custom_command(
    OUTPUT ${OBJ_FILE}
    COMMAND ${CMAKE_COMMAND} ARGS -E copy $<TARGET_FILE:${NAME}> ${BIN_FILE}
    COMMAND
      ${CMAKE_OBJCOPY} ARGS -I binary -O elf32-i386 -B i386 --rename-section
      .data=.rodata,alloc,load,readonly,data,contents ${NAME}_bin ${OBJ_FILE}
    DEPENDS ${NAME}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

  add_custom_target(${NAME}_embed DEPENDS ${OBJ_FILE})

  list(APPEND USER_PROGRAM_OBJECTS ${OBJ_FILE})
  list(APPEND USER_PROGRAM_TARGETS ${NAME}_embed)
  set(USER_PROGRAM_OBJECTS
      "${USER_PROGRAM_OBJECTS}"
      PARENT_SCOPE)
  set(USER_PROGRAM_TARGETS
      "${USER_PROGRAM_TARGETS}"
      PARENT_SCOPE)
endfunction()

file(GLOB user_srcs src/user/entry/*.cpp)

foreach(user_src ${user_srcs})
  get_filename_component(user_prog_name ${user_src} NAME_WLE)
  message("Add user program ${user_src} as ${user_prog_name}")
  add_user_program(${user_prog_name} ${user_src})
endforeach()

# kernel

set(syscall_srcs
    src/syscall/exit.cpp
    src/syscall/spawn.cpp
    src/syscall/read.cpp
    src/syscall/write.cpp
    src/syscall/waitpid.cpp
    src/syscall/getpid.cpp
    src/syscall/kill.cpp
    src/syscall/brk.cpp
    src/syscall/signal.cpp
    src/syscall/sigreturn.cpp
    src/syscall/nanosleep.cpp)

add_executable(
  kernel
  src/main.cpp
  src/boot.s
  src/allocator/load.cpp
  src/allocator/slab.cpp
  src/allocator/utils.cpp
  src/data/embed.cpp
  src/gdt/load.cpp
  src/interrupt/impl.cpp
  src/interrupt/isr.cpp
  src/interrupt/load.cpp
  src/interrupt/syscall.s
  src/keyboard/load.cpp
  src/keyboard/message.cpp
  src/paging/directory.cpp
  src/paging/kernel.cpp
  src/task/switch.s
  src/task/trampoline.s
  src/task/pid.cpp
  src/task/task.cpp
  src/task/tcb.cpp
  src/task/wait.cpp
  src/timer/load.cpp
  src/console/buffer.cpp
  src/console/entry.cpp
  src/lib/libcxx.cpp
  src/lib/libc.cpp
  ${syscall_srcs})
target_link_options(kernel PRIVATE -T ${CMAKE_CURRENT_LIST_DIR}/src/linker.ld)

target_compile_options(kernel PRIVATE -g -Wall -Wextra)
target_compile_definitions(kernel PRIVATE __BUILDING_NYAN__)

if(NYAN_ENABLE_OPTIMIZE)
  target_compile_options(
    kernel PRIVATE -flto=thin -funified-lto -O2 -fomit-frame-pointer
                   -ffunction-sections -fdata-sections)
  target_link_options(kernel PRIVATE --lto=thin --gc-sections)
else()
  target_compile_options(kernel PRIVATE -Og)

endif()

# 不能开启向量化, 会产生内存引用, 提前启用sse也不行
set_source_files_properties(src/paging/kernel.cpp PROPERTIES COMPILE_FLAGS
                                                             "-mno-sse")
set_source_files_properties(src/interrupt/isr.cpp
                            PROPERTIES COMPILE_FLAGS "-mgeneral-regs-only")

set_target_properties(kernel PROPERTIES LINK_DEPENDS
                                        ${CMAKE_CURRENT_LIST_DIR}/src/linker.ld)

foreach(target ${USER_PROGRAM_TARGETS})
  add_dependencies(kernel ${target})
endforeach()

target_link_directories(kernel PRIVATE sysroot/usr/lib/generic
                        sysroot/usr/lib/i686-none-elf)
target_link_libraries(kernel PRIVATE kernel_header c clang_rt.builtins-i386
                                     ${USER_PROGRAM_OBJECTS})
