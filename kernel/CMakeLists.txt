# user program

file(GLOB syscall_srcs CONFIGURE_DEPENDS syscall/*.cpp)

add_executable(
  kernel
  main.cpp
  boot.s
  allocator/load.cpp
  allocator/slab.cpp
  allocator/utils.cpp
  gdt/load.cpp
  interrupt/impl.cpp
  interrupt/isr.cpp
  interrupt/load.cpp
  interrupt/syscall.s
  keyboard/load.cpp
  keyboard/message.cpp
  paging/directory.cpp
  paging/kernel.cpp
  task/switch.s
  task/trampoline.s
  task/fd.cpp
  task/pid.cpp
  task/task.cpp
  task/tcb.cpp
  task/wait.cpp
  timer/load.cpp
  console/buffer.cpp
  console/entry.cpp
  lib/clang.cpp
  lib/libcxx.cpp
  lib/libc.cpp
  ${syscall_srcs})

# 不能开启向量化, 会产生内存引用, 提前启用sse也不行
set_source_files_properties(paging/kernel.cpp PROPERTIES COMPILE_FLAGS
                                                         "-mno-sse")
set_source_files_properties(interrupt/isr.cpp PROPERTIES COMPILE_FLAGS
                                                         "-mgeneral-regs-only")
set_target_properties(kernel PROPERTIES LINK_DEPENDS
                                        ${CMAKE_CURRENT_LIST_DIR}/linker.ld)

target_compile_options(kernel PRIVATE -g -Wall -Wextra)
target_compile_definitions(kernel PRIVATE __BUILDING_NYAN__)

target_link_options(kernel PRIVATE -T ${CMAKE_CURRENT_LIST_DIR}/linker.ld)
target_link_directories(kernel PRIVATE ../sysroot/usr/lib/generic
                        ../sysroot/usr/lib/i686-none-elf)
target_link_libraries(kernel PRIVATE kernel_header kernel_embed c
                                     clang_rt.builtins-i386)

if(NYAN_ENABLE_OPTIMIZE)
  target_compile_options(
    kernel PRIVATE -flto=thin -funified-lto -O2 -fomit-frame-pointer
                   -ffunction-sections -fdata-sections)
  target_link_options(kernel PRIVATE --lto=thin --gc-sections)
else()
  target_compile_options(kernel PRIVATE -Og)
endif()
