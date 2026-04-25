#!/bin/bash

cmake -S nyan-env/llvm-project/llvm -B build-clang -G Ninja \
  -DCMAKE_C_COMPILER=clang-22 -DCMAKE_CXX_COMPILER=clang++-22 \
  -DLLVM_ENABLE_PROJECTS="clang;lld;clang-tools-extra" \
  -DCMAKE_BUILD_TYPE=Release -DLLVM_DEFAULT_TARGET_TRIPLE=i686-unknown-linux \
  -DLLVM_TARGETS_TO_BUILD=X86 \
  -DLLVM_ENABLE_LLD=ON -DLLVM_ENABLE_LTO=Thin \
  -DLLVM_BUILD_LLVM_DYLIB=ON -DLLVM_LINK_LLVM_DYLIB=ON \
  -DLLVM_INSTALL_TOOLCHAIN_ONLY=ON \
  -DCLANG_DEFAULT_LINKER=lld \
  -DCLANG_DEFAULT_CXX_STDLIB=libc++ \
  -DCLANG_DEFAULT_RTLIB=compiler-rt \
  -DCLANG_DEFAULT_UNWINDLIB=libunwind

cmake --build build-clang
sudo cmake --install build-clang --prefix /opt/llvm-22