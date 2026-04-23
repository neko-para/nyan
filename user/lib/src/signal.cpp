#include <nyan/syscall.h>
#include <signal.h>
#include <unistd.h>

#include "utils.hpp"

extern "C" {

sighandler_t signal(int sig, sighandler_t handler) {
    return reinterpret_cast<sighandler_t>(wrapRet(reinterpret_cast<intptr_t>(sys_signal(sig, handler))));
}

int kill(pid_t pid, int sig) {
    return wrapRet(sys_kill(pid, sig));
}

int raise(int sig) {
    return kill(getpid(), sig);
}
}
