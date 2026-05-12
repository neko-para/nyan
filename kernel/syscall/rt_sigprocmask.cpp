#include <nyan/syscall.h>
#include <signal.h>

#include "../task/mod.hpp"
#include "../task/scheduler.hpp"
#include "../task/tcb.hpp"

namespace nyan::syscall {

int rt_sigprocmask(int how, const __nyan_sigset* nset, __nyan_sigset* oset, size_t sigsetsize) {
    if (sigsetsize != sizeof(__nyan_sigset)) {
        return SYS_EINVAL;
    }
    __try
        (task::checkR(nset, 1, true));
    __try
        (task::checkW(oset, 1, true));

    if (oset) {
        *oset = task::__scheduler->__current->__signal.__signal_mask;
    }
    if (!nset) {
        return 0;
    }
    switch (how) {
        case SIG_BLOCK:
            task::__scheduler->__current->__signal.__signal_mask |= *nset;
            return 0;
        case SIG_UNBLOCK:
            task::__scheduler->__current->__signal.__signal_mask &= ~*nset;
            return 0;
        case SIG_SETMASK:
            task::__scheduler->__current->__signal.__signal_mask = *nset;
            return 0;
        default:
            return SYS_EINVAL;
    }
}

}  // namespace nyan::syscall
