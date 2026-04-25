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
        task::currentTask->signalActions.reset(allocator::allocAs<task::SigActionData>());
    }
    auto& entry = task::currentTask->signalActions->operator[](sig);
    auto old = std::move(entry);
    entry.reset(new struct sigaction);
    entry->sa_handler = handler;
    entry->sa_mask.__bits[0] = 1u << sig;
    entry->sa_flags = SA_RESTART;
    entry->sa_restorer = 0;
    return old->sa_handler;
}

}  // namespace nyan::syscall
