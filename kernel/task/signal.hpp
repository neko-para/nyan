#pragma once

#include "../interrupt/isr.hpp"

namespace nyan::task {

using SigSet = uint64_t;

struct SignalFrame {
    uint32_t retAddr;
    int signal;
    SigSet oldMask;
    interrupt::SyscallFrame frame;
};

struct SigAction {
    void (*__handler)(int);
    SigSet __mask;
    int __flags;
};

}  // namespace nyan::task
