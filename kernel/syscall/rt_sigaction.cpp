
#include <nyan/syscall.h>

#include "../task/mod.hpp"
#include "../task/scheduler.hpp"
#include "../task/tcb.hpp"

namespace nyan::syscall {

int rt_sigaction(int sig, const struct sigaction* act, struct sigaction* oldact, size_t sigsetsize) {
    if (sigsetsize != 8) {
        return SYS_EINVAL;
    }
    __try
        (task::checkR(act, 1, true));
    __try
        (task::checkW(oldact, 1, true));

    if (sig < 1 || sig >= NSIG || sig == SIGKILL || sig == SIGSTOP) {
        return SYS_EINVAL;
    }

    // 64暂时也放不下
    if (sig == 64) {
        return SYS_EINVAL;
    }

    // 如果要设置新动作, 先检查不支持的特性
    if (act) {
        if (act->sa_flags & SA_SIGINFO) {
            return SYS_EINVAL;
        }

        if (act->sa_flags & SA_RESTORER) {
            return SYS_EINVAL;
        }

        if (act->sa_handler != SIG_DFL && act->sa_handler != SIG_IGN) {
            __try
                (task::checkE(act->sa_handler));
        }
    }

    if (!task::__scheduler->__current->__signal.__signal_actions) {
        if (!act && !oldact) {
            return 0;
        }
        task::__scheduler->__current->__signal.ensureActions();
    }

    auto& entry = task::__scheduler->__current->__signal.__signal_actions->operator[](sig);

    // 输出旧的 sigaction
    if (oldact) {
        sigset_t sigset{};
        sigset.__bits[0] = entry.__mask & 0xFFFFFFFF;
        sigset.__bits[1] = (entry.__mask >> 32) & 0xFFFFFFFF;

        oldact->sa_handler = entry.__handler;
        oldact->sa_mask = sigset;
        oldact->sa_flags = entry.__flags;
        oldact->sa_restorer = nullptr;
    }

    if (act) {
        uint64_t mask = act->sa_mask.__bits[0] | (static_cast<uint64_t>(act->sa_mask.__bits[1]) << 32);
        if (!(act->sa_flags & SA_NODEFER)) {
            mask |= 1ull << sig;
        }

        entry.__handler = act->sa_handler;
        entry.__mask = mask;
        entry.__flags = act->sa_flags;
    }

    return 0;
}

}  // namespace nyan::syscall
