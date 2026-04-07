cmake_minimum_required(VERSION 3.30)

project(nyan CXX ASM)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(kernel src/main.cpp src/setup/paging.cpp src/vga/print.cpp
                      src/boot.s)
target_link_options(kernel PRIVATE -T ${CMAKE_CURRENT_LIST_DIR}/src/linker.ld)
