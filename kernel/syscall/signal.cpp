#include <nyan/syscall.h>

#include "../allocator/alloc.hpp"
#include "../arch/guard.hpp"
#include "../task/tcb.hpp"

namespace nyan::syscall {

sighandler_t signal(int sig, sighandler_t handler) {
    if (sig < 0 || sig >= NSIG || sig == SIGKILL || sig == SIGSTOP) {
        return reinterpret_cast<sighandler_t>(-SYS_EINVAL);
    }
    arch::InterruptGuard guard;
    if (!task::currentTask->signalActions) {
        task::currentTask->signalActions.reset(allocator::allocAs<std::array<sigaction, NSIG>>());
    }
    auto& entry = task::currentTask->signalActions->operator[](sig);
    auto old = entry;
    entry = {
        handler,
        1u << sig,
        SA_RESTART,
    };
    return old.sa_handler;
}

}  // namespace nyan::syscall
