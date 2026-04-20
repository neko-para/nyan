#include <nyan/syscall.h>

#include "../task/signal.hpp"
#include "../task/tcb.hpp"

namespace nyan::syscall {

void sigreturn(void* frame) {
    interrupt::SyscallFrame* sysFrame = static_cast<interrupt::SyscallFrame*>(frame);

    auto esp = sysFrame->user_esp;
    esp += 4;  // signum
    task::currentTask->signalMask = *reinterpret_cast<sigset_t*>(esp);
    esp += 4;  // sigmask
    auto userFrame = reinterpret_cast<interrupt::SyscallFrame*>(esp);
    *sysFrame = *userFrame;
}

}  // namespace nyan::syscall
