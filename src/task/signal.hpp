#pragma once

#include <sys/signal.h>

#include "../interrupt/isr.hpp"

namespace nyan::task {

struct SignalFrame {
    uint32_t retAddr;
    int signal;
    sigset_t oldMask;
    interrupt::SyscallFrame frame;
};

}  // namespace nyan::task
