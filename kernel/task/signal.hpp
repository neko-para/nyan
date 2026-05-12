#pragma once

#include <nyan/signal.h>

#include "../interrupt/isr.hpp"

namespace nyan::task {

struct SignalFrame {
    uint32_t retAddr;
    int signal;
    __nyan_sigset oldMask;
    interrupt::SyscallFrame frame;
};

}  // namespace nyan::task
