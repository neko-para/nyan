#!/bin/bash

. ./i686-elf

SYSROOT=`pwd`/sysroot

mkdir -p build-musl
cd build-musl
../nyan-env/musl/configure --prefix="$SYSROOT/usr" --disable-shared
make -j 16 install-headers