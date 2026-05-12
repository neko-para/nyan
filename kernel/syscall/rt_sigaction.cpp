
#include <nyan/syscall.h>

#include "../task/mod.hpp"
#include "../task/scheduler.hpp"
#include "../task/tcb.hpp"

namespace nyan::syscall {

int rt_sigaction(int sig, const __nyan_sigaction* act, struct __nyan_sigaction* oact, size_t sigsetsize) {
    if (sigsetsize != sizeof(__nyan_sigset)) {
        return SYS_EINVAL;
    }
    __try
        (task::checkR(act, 1, true));
    __try
        (task::checkW(oact, 1, true));

    if (sig < 1 || sig >= NSIG || sig == SIGKILL || sig == SIGSTOP) {
        return SYS_EINVAL;
    }

    if (act) {
        if (act->__flags & SA_SIGINFO) {
            return SYS_EINVAL;
        }

        if (act->__handler != SIG_DFL && act->__handler != SIG_IGN) {
            __try
                (task::checkE(act->__handler));
        }
    }

    if (!task::__scheduler->__current->__signal.__signal_actions) {
        if (!act && !oact) {
            return 0;
        }
        task::__scheduler->__current->__signal.ensureActions();
    }

    auto& entry = task::__scheduler->__current->__signal.__signal_actions->operator[](sig);

    if (oact) {
        *oact = entry;
    }

    if (act) {
        __nyan_sigset mask = act->__mask;
        if (!(act->__flags & SA_NODEFER)) {
            mask |= 1ull << (sig - 1);
        }

        entry.__handler = act->__handler;
        entry.__mask = mask;
        entry.__flags = act->__flags;
        entry.__restorer = act->__restorer;
    }

    return 0;
}

}  // namespace nyan::syscall
