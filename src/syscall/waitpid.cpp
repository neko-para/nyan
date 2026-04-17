#include <nyan/syscall.h>

#include <sys/wait.h>

#include "../task/guard.hpp"
#include "../task/pid.hpp"
#include "../task/task.hpp"
#include "../task/tcb.hpp"

namespace nyan::syscall {

constexpr int KnownOptions = WNOHANG;

pid_t waitpid(pid_t pid, int* stat_loc, int options) {
    if ((options & KnownOptions) != options) {
        return -SYS_EINVAL;
    }

    if (pid < -1) {
        // wait group
        return -SYS_ENOSYS;
    } else if (pid == -1) {
        // wait any child
        return -SYS_ENOSYS;
    } else if (pid == 0) {
        // wait any child in same group
        return -SYS_ENOSYS;
    } else {
        // wait pid
        while (true) {
            task::InterruptGuard guard;
            auto tcb = task::findTask(pid);
            if (!tcb) {
                return -SYS_ECHILD;
            }

            if (!tcb->ended()) {
                if (options & WNOHANG) {
                    return 0;
                }

                tcb->waitingTasks.pushFront(task::currentTask.head);
                task::currentTask->waitInfo = {pid};
                task::block(task::BlockReason::BR_WaitTask);
            } else {
                if (stat_loc) {
                    // TODO: signal
                    *stat_loc = ((tcb->exitInfo.code & 0xFF) << 8) | 0;
                }
                task::freeTask(pid, 0);
                return pid;
            }
        }
    }
}

}  // namespace nyan::syscall
