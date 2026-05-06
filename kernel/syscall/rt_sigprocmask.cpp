#include <nyan/syscall.h>
#include <signal.h>

#include "../task/scheduler.hpp"
#include "../task/tcb.hpp"
#include "utils.hpp"

namespace nyan::syscall {

int rt_sigprocmask(int how, const sigset_t* set, sigset_t* oldset, size_t sigsetsize) {
    if (sigsetsize != sizeof(sigset_t)) {
        return SYS_EINVAL;
    }
    if (!utils::validateReadAuto(set, 1, true) || !utils::validateWriteAuto(oldset, 1, true)) {
        return SYS_EFAULT;
    }

    if (oldset) {
        oldset->__bits[0] = task::__scheduler->__current->__signal.__signal_mask & 0xFFFFFFFF;
        oldset->__bits[1] = (task::__scheduler->__current->__signal.__signal_mask >> 32) & 0xFFFFFFFF;
    }
    if (!set) {
        return 0;
    }
    switch (how) {
        case SIG_BLOCK:
            task::__scheduler->__current->__signal.__signal_mask |= set->__bits[0];
            task::__scheduler->__current->__signal.__signal_mask |= static_cast<uint64_t>(set->__bits[1]) << 32;
            return 0;
        case SIG_UNBLOCK:
            task::__scheduler->__current->__signal.__signal_mask &= ~set->__bits[0];
            task::__scheduler->__current->__signal.__signal_mask &= static_cast<uint64_t>(~set->__bits[1]) << 32;
            return 0;
        case SIG_SETMASK:
            task::__scheduler->__current->__signal.__signal_mask =
                static_cast<uint64_t>(set->__bits[0]) | (static_cast<uint64_t>(set->__bits[1]) << 32);
            return 0;
        default:
            return SYS_EINVAL;
    }
}

}  // namespace nyan::syscall
