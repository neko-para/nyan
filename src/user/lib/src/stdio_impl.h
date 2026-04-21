#pragma once

#include <stdio.h>

struct FILE {
    int fd;
};

extern "C" FILE __stdin_obj;
extern "C" FILE __stdout_obj;
extern "C" FILE __stderr_obj;

extern "C" void __init_stdio();
