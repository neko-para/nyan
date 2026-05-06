#include <nyan/syscall.h>
#include <string.h>

#include "utils.hpp"

namespace nyan::syscall {

sighandler_t signal(int sig, sighandler_t handler) {
    if (!utils::validateExec(handler)) {
        return reinterpret_cast<sighandler_t>(-SYS_EFAULT);
    }

    struct sigaction act;
    struct sigaction oldact;

    sigset_t sigset{};
    if (sig <= 32) {
        sigset.__bits[0] = 1u << sig;
    } else if (sig <= 64) {
        sigset.__bits[1] = 1u << (sig - 32);
    } else {
        return reinterpret_cast<sighandler_t>(-SYS_EINVAL);
    }

    memset(&act, 0, sizeof(act));
    act.sa_handler = handler;
    act.sa_mask = sigset;
    act.sa_flags = SA_RESTART;

    int ret = rt_sigaction(sig, &act, &oldact, sizeof(sigset_t));
    if (ret < 0) {
        return reinterpret_cast<sighandler_t>(ret);
    }
    return oldact.sa_handler;
}

}  // namespace nyan::syscall
