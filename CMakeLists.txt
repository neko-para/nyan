cmake_minimum_required(VERSION 3.30)

project(nyan CXX ASM)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(NYAN_ENABLE_OPTIMIZE CACHE OFF)

add_library(kernel_header INTERFACE)
target_include_directories(kernel_header INTERFACE include)

add_subdirectory(user)

add_subdirectory(kernel)
