#include <nyan/syscall.h>
#include <signal.h>

#include "../task/tcb.hpp"

namespace nyan::syscall {

int rt_sigprocmask(int how, const sigset_t* set, sigset_t* oldset, size_t) {
    if (oldset) {
        oldset->__bits[0] = task::currentTask->signalMask & 0xFFFFFFFF;
        oldset->__bits[1] = (task::currentTask->signalMask >> 32) & 0xFFFFFFFF;
    }
    if (!set) {
        return 0;
    }
    switch (how) {
        case SIG_BLOCK:
            task::currentTask->signalMask |= set->__bits[0];
            task::currentTask->signalMask |= static_cast<uint64_t>(set->__bits[1]) << 32;
            return 0;
        case SIG_UNBLOCK:
            task::currentTask->signalMask &= ~set->__bits[0];
            task::currentTask->signalMask &= static_cast<uint64_t>(~set->__bits[1]) << 32;
            return 0;
        case SIG_SETMASK:
            task::currentTask->signalMask =
                static_cast<uint64_t>(set->__bits[0]) | (static_cast<uint64_t>(set->__bits[1]) << 32);
            return 0;
        default:
            return -SYS_EINVAL;
    }
}

}  // namespace nyan::syscall
