#!/bin/bash

PROJ_ROOT=`dirname "$0"`/..
PROJ_ROOT=`realpath "$PROJ_ROOT"`
qemu-system-i386 -kernel "$PROJ_ROOT/build/kernel_install/kernel" -debugcon stdio -m 2G
