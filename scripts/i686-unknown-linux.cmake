set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR x86)

set(TOOL_PREFIX /opt/llvm-22)

set(CMAKE_C_COMPILER_TARGET i686-unknown-linux)
set(CMAKE_C_COMPILER ${TOOL_PREFIX}/bin/clang)
set(CMAKE_CXX_COMPILER_TARGET i686-unknown-linux)
set(CMAKE_CXX_COMPILER ${TOOL_PREFIX}/bin/clang++)
set(CMAKE_ASM_COMPILER_TARGET i686-unknown-linux)
set(CMAKE_ASM_COMPILER ${TOOL_PREFIX}/bin/clang)
set(CMAKE_OBJCOPY ${TOOL_PREFIX}/bin/llvm-objcopy)
set(CMAKE_LINKER ${TOOL_PREFIX}/bin/ld.lld)

set(CMAKE_SYSROOT "${CMAKE_CURRENT_LIST_DIR}/sysroot")

set(CMAKE_C_FLAGS "-ffreestanding -nostdlib -D_GNU_SOURCE")
set(CMAKE_CXX_FLAGS "-ffreestanding -nostdlib -D_GNU_SOURCE")

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
