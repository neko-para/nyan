#include <nyan/syscall.h>

#include "../task/pid.hpp"
#include "../task/tcb.hpp"

namespace nyan::syscall {

int setpgid(pid_t pid, pid_t pgid) {
    auto tcb = task::findTask(pid);
    if (!tcb) {
        return SYS_ESRCH;
    }
    // TODO: lots of checks
    tcb->groupPid = pgid;
    return 0;
}

}  // namespace nyan::syscall
