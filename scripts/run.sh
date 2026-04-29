#!/bin/bash

PROJ_ROOT="`dirname "$0"`/.."
PROJ_ROOT="`realpath "$PROJ_ROOT"`"
LOG_FILE="${PROJ_ROOT}/scripts/nyan.log"
LOG_FILE_BAK="${PROJ_ROOT}/scripts/nyan.log.bak"
KERNEL_FILE="$PROJ_ROOT/build/kernel_install/kernel"

if [[ -e "$LOG_FILE" ]]; then
    mv $LOG_FILE $LOG_FILE_BAK
fi
touch $LOG_FILE

node $PROJ_ROOT/logger/watch.ts "$LOG_FILE" "$KERNEL_FILE" &
qemu-system-i386 -kernel "$KERNEL_FILE" -debugcon "file:$LOG_FILE" -m 2G $@
