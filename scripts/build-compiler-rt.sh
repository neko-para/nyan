#!/bin/bash

cmake -S nyan-env/llvm-project/runtimes -B build-compiler-rt -G "Unix Makefiles" \
  --toolchain `pwd`/i686-elf.cmake \
  -DCMAKE_BUILD_TYPE=Release \
  -DLLVM_DEFAULT_TARGET_TRIPLE=i686-unknown-linux -DCOMPILER_RT_DEFAULT_TARGET_ONLY=ON \
  -DLLVM_ENABLE_RUNTIMES="compiler-rt" \
  -DCOMPILER_RT_BUILD_SANITIZERS=OFF \
  -DCOMPILER_RT_BUILD_XRAY=OFF \
  -DCOMPILER_RT_BUILD_LIBFUZZER=OFF \
  -DCOMPILER_RT_BUILD_PROFILE=OFF \
  -DCOMPILER_RT_BUILD_CTX_PROFILE=OFF \
  -DCOMPILER_RT_BUILD_MEMPROF=OFF \
  -DCOMPILER_RT_BUILD_ORC=OFF \
  -DCOMPILER_RT_BUILD_GWP_ASAN=OFF \
  -DCOMPILER_RT_BAREMETAL_BUILD=ON

cmake --build build-compiler-rt -- -j 16
sudo cmake --install build-compiler-rt --prefix /opt/llvm-22/lib/clang/22