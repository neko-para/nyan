#pragma once

#include <stdint.h>

namespace nyan::logger {

enum Type : uint8_t {
    T_Log,
    T_Syscall,
};

enum LogLevel : uint8_t {
    LL_Debug,
    LL_Info,
    LL_Warn,
    LL_Error,
    LL_Fatal,
};

enum SyscallRole : uint8_t {
    SR_Enter,
    SR_Leave,
};

struct Payload {
    uint32_t ts;
    uint32_t eip;
    int32_t pid;
    uint16_t len;
    Type type;
    union {
        LogLevel logLevel;
        SyscallRole syscallRole;
    };
};

struct SyscallContent {
    uint32_t ret;
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t esi;
    uint32_t edi;
    uint32_t ebp;
};

}  // namespace nyan::logger
