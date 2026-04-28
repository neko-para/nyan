cmake_minimum_required(VERSION 3.30)

project(nyan NONE)

include(ExternalProject)

option(NYAN_ENABLE_OPTIMIZE CACHE OFF)

if(APPLE)
  set(NYAN_PLATFORM mac)
elseif(LINUX)
  set(NYAN_PLATFORM linux)
else()
  set(NYAN_PLATFORM win)
endif()

set(KERNEL_TOOLCHAIN
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/toolchain-kernel-${NYAN_PLATFORM}.cmake")
set(USER_TOOLCHAIN
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/toolchain-user-${NYAN_PLATFORM}.cmake")

set(USER_INSTALL_DIR ${CMAKE_BINARY_DIR}/user_install)
set(KERNEL_INSTALL_DIR ${CMAKE_BINARY_DIR}/kernel_install)

ExternalProject_Add(
  user_programs
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/user
  CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=${USER_TOOLCHAIN}
             -DCMAKE_INSTALL_PREFIX=${USER_INSTALL_DIR}
             -DNYAN_ENABLE_OPTIMIZE=${NYAN_ENABLE_OPTIMIZE}
  BUILD_ALWAYS ON
  USES_TERMINAL_CONFIGURE 1
  USES_TERMINAL_BUILD 1
  USES_TERMINAL_INSTALL 1)
ExternalProject_Get_Property(user_programs BINARY_DIR)
set(USER_BUILD_DIR ${BINARY_DIR})

ExternalProject_Add(
  kernel_build
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/kernel
  CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=${KERNEL_TOOLCHAIN}
             -DNYAN_ENABLE_OPTIMIZE=${NYAN_ENABLE_OPTIMIZE}
             -DNYAN_USER_INSTALL_DIR=${USER_INSTALL_DIR}
             -DCMAKE_INSTALL_PREFIX=${KERNEL_INSTALL_DIR}
  BUILD_ALWAYS ON
  DEPENDS user_programs
  USES_TERMINAL_CONFIGURE 1
  USES_TERMINAL_BUILD 1
  USES_TERMINAL_INSTALL 1)
ExternalProject_Get_Property(kernel_build BINARY_DIR)
set(KERNEL_BUILD_DIR ${BINARY_DIR})

find_program(COMPDB compdb HINTS ${CMAKE_SOURCE_DIR}/.venv/bin)

if(COMPDB)
  add_custom_target(
    compile_commands ALL
    COMMAND ${COMPDB} -p ${USER_BUILD_DIR} -p ${KERNEL_BUILD_DIR} list >
            ${CMAKE_BINARY_DIR}/compile_commands.json
    DEPENDS kernel_build user_programs)
endif()
