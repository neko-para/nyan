#pragma once

#include <stdio.h>

enum FileFlags {
    F_Err = 1 << 0,
    F_Eof = 1 << 1,
    F_Out = 1 << 2,

    F_FullBuf = 0 << 3,
    F_LineBuf = 1 << 3,
    F_NoBuf = 2 << 3,
    F_BufMask = 3 << 3,
};

struct FILE {
    int fd;
    int flags;

    char* buf;
    size_t buf_size;
    size_t buf_pos;

    int bufMode() const noexcept { return flags & F_BufMask; }
    bool isBufFull() const noexcept { return buf_pos == buf_size; }
};

extern "C" FILE __stdin_obj;
extern "C" FILE __stdout_obj;
extern "C" FILE __stderr_obj;

extern "C" void __init_stdio();
extern "C" void __fini_stdio();
