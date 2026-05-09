#include <nyan/syscall.h>

#include "../task/pid.hpp"
#include "../task/scheduler.hpp"
#include "../task/tcb.hpp"

namespace nyan::syscall {

int setpgid(pid_t pid, pid_t pgid) {
    // TODO: lots of checks
    if (pid) {
        __try
            (task::findTaskNew(pid))->groupPid = pgid;
    } else {
        task::__scheduler->__current->groupPid = pgid;
    }
    return 0;
}

}  // namespace nyan::syscall
