cmake_minimum_required(VERSION 3.30)

project(nyan CXX ASM)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(
  kernel
  src/main.cpp
  src/allocator/frame.cpp
  src/allocator/pool.cpp
  src/allocator/utils.cpp
  src/gdt/load.cpp
  src/interrupt/isr.cpp
  src/interrupt/load.cpp
  src/keyboard/load.cpp
  src/keyboard/message.cpp
  src/paging/kernel.cpp
  src/setup/paging.cpp
  src/timer/load.cpp
  src/vga/print.cpp
  src/boot.s
  src/lib/libcxx.cpp
  src/lib/libc.cpp)
target_link_options(kernel PRIVATE -T ${CMAKE_CURRENT_LIST_DIR}/src/linker.ld)

target_compile_options(kernel PRIVATE -g -Wall -Wextra)

target_compile_options(kernel PRIVATE -flto=thin -funified-lto -O2
                                      -fomit-frame-pointer)
target_link_options(kernel PRIVATE --lto=thin)

# 不能开启向量化, 会产生内存引用, 提前启用sse也不行
set_source_files_properties(src/setup/paging.cpp PROPERTIES COMPILE_FLAGS
                                                            "-mno-sse")
set_source_files_properties(src/interrupt/isr.cpp
                            PROPERTIES COMPILE_FLAGS "-mgeneral-regs-only")

set_target_properties(kernel PROPERTIES LINK_DEPENDS
                                        ${CMAKE_CURRENT_LIST_DIR}/src/linker.ld)

target_link_directories(kernel PRIVATE sysroot/usr/lib/generic
                        sysroot/usr/lib/i686-none-elf)
target_link_libraries(kernel PRIVATE c clang_rt.builtins-i386)
