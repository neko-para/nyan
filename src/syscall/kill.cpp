#include <nyan/syscall.h>

#include "../task/pid.hpp"
#include "../task/task.hpp"

namespace nyan::syscall {

int sys_kill(pid_t pid, int sig) {
    if (sig >= NSIG) {
        return -SYS_EINVAL;
    }
    if (pid > 0) {
        auto tcb = task::findTask(pid);
        if (!tcb) {
            return -SYS_ESRCH;
        } else {
            // TODO: check permission
            if (sig == 0) {
                return 0;
            }
            sendSignal(tcb, sig);
            return 0;
        }
    } else if (pid == 0) {
        // TODO: send to current process group
        return -SYS_ENOSYS;
    } else if (pid == -1) {
        // TODO: send to everyone if possible
        return -SYS_ENOSYS;
    } else {
        // TODO: send to certain process group
        return -SYS_ENOSYS;
    }
}

}  // namespace nyan::syscall
