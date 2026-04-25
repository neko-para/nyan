#pragma once

#include <signal.h>

#include "../interrupt/isr.hpp"

namespace nyan::task {

struct SignalFrame {
    uint32_t retAddr;
    int signal;
    uint32_t oldMask;
    interrupt::SyscallFrame frame;
};

}  // namespace nyan::task
