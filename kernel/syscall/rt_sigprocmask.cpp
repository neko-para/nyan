#include <nyan/syscall.h>
#include <signal.h>

#include "../task/mod.hpp"
#include "../task/scheduler.hpp"
#include "../task/tcb.hpp"

namespace nyan::syscall {

int rt_sigprocmask(int how, const sigset_t* nset, sigset_t* oset, size_t sigsetsize) {
    if (sigsetsize != 8) {
        return SYS_EINVAL;
    }
    __try
        (task::checkR(nset, 1, true));
    __try
        (task::checkW(oset, 1, true));

    if (oset) {
        oset->__bits[0] = task::__scheduler->__current->__signal.__signal_mask & 0xFFFFFFFF;
        oset->__bits[1] = (task::__scheduler->__current->__signal.__signal_mask >> 32) & 0xFFFFFFFF;
    }
    if (!nset) {
        return 0;
    }
    switch (how) {
        case SIG_BLOCK:
            task::__scheduler->__current->__signal.__signal_mask |= nset->__bits[0];
            task::__scheduler->__current->__signal.__signal_mask |= static_cast<uint64_t>(nset->__bits[1]) << 32;
            return 0;
        case SIG_UNBLOCK:
            task::__scheduler->__current->__signal.__signal_mask &= ~nset->__bits[0];
            task::__scheduler->__current->__signal.__signal_mask &= static_cast<uint64_t>(~nset->__bits[1]) << 32;
            return 0;
        case SIG_SETMASK:
            task::__scheduler->__current->__signal.__signal_mask =
                static_cast<uint64_t>(nset->__bits[0]) | (static_cast<uint64_t>(nset->__bits[1]) << 32);
            return 0;
        default:
            return SYS_EINVAL;
    }
}

}  // namespace nyan::syscall
