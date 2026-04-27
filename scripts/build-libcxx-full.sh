#!/bin/bash

cmake -S nyan-env/llvm-project/runtimes -B build-libcxx-full -G "Unix Makefiles" \
  --toolchain `pwd`/i686-unknown-linux.cmake \
  -DCMAKE_BUILD_TYPE=Release \
  -DLLVM_DEFAULT_TARGET_TRIPLE=i686-unknown-linux -DCOMPILER_RT_DEFAULT_TARGET_ONLY=ON \
  -DLLVM_ENABLE_RUNTIMES="libcxx;libcxxabi;libunwind;compiler-rt" \
  -DLIBCXX_ENABLE_SHARED=OFF \
  -DLIBCXX_CXX_ABI=libcxxabi \
  -DLIBCXX_HAS_MUSL_LIBC=ON \
  -DCOMPILER_RT_BUILD_SANITIZERS=OFF \
  -DCOMPILER_RT_BUILD_XRAY=OFF \
  -DCOMPILER_RT_BUILD_LIBFUZZER=OFF \
  -DCOMPILER_RT_BUILD_PROFILE=OFF \
  -DCOMPILER_RT_BUILD_CTX_PROFILE=OFF \
  -DCOMPILER_RT_BUILD_MEMPROF=OFF \
  -DCOMPILER_RT_BUILD_ORC=OFF \
  -DCOMPILER_RT_BUILD_GWP_ASAN=OFF \
  -DLIBUNWIND_ENABLE_SHARED=OFF \
  -DLIBUNWIND_USE_COMPILER_RT=ON \
  -DLIBCXXABI_ENABLE_SHARED=OFF \
  -DLIBCXXABI_USE_LLVM_UNWINDER=ON \
  -DLIBCXXABI_USE_COMPILER_RT=ON

cmake --build build-libcxx-full -- -j 16
cmake --install build-libcxx-full --prefix `pwd`/libcxx-full