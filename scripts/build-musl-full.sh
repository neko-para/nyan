#!/bin/bash

. ./i686-elf

SYSROOT=`pwd`/sysroot
export LIBCC=/opt/llvm-22/lib/clang/22/lib/generic/libclang_rt.builtins-i386.a

mkdir -p build-musl-full
cd build-musl-full
../nyan-env/musl/configure --prefix="$SYSROOT/usr" --disable-shared
make -j 16
make install -j 16
cd ..

export CFLAGS="-g -Og"
mkdir -p build-musl-full-debug
cd build-musl-full-debug
../nyan-env/musl/configure --prefix="$SYSROOT/usr" --disable-shared --libdir="$SYSROOT/usr/lib/debug"
make -j 16
make install -j 16
cd ..

