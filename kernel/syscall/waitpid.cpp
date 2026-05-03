#include <nyan/syscall.h>

#include <sys/wait.h>

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
        while (true) {
            if (task::currentTask->childTasks.empty()) {
                return -SYS_ECHILD;
            }
            for (auto& tcb : task::currentTask->childTasks) {
                if (tcb.ended()) {
                    auto findPid = tcb.pid;
                    task::freeTask(findPid, stat_loc);
                    return findPid;
                }
            }

            if (options & WNOHANG) {
                return 0;
            }

            task::currentTask->waitTaskInfo = {pid};
            if (block(task::BlockReason::BR_WaitTask) == task::WakeReason::WR_Signal) {
                if (task::currentTask->peekSignal()) {
                    return -SYS_EINTR;
                }
            }
        }
    } else if (pid == 0) {
        // wait any child in same group
        return -SYS_ENOSYS;
    } else {
        // wait pid
        while (true) {
            auto tcb = task::findTask(pid);
            if (!tcb || tcb->parentPid != task::currentTask->pid) {
                return -SYS_ECHILD;
            }

            if (!tcb->ended()) {
                if (options & WNOHANG) {
                    return 0;
                }

                task::currentTask->waitTaskInfo = {pid};
                if (task::block(task::BlockReason::BR_WaitTask) == task::WakeReason::WR_Signal) {
                    if (task::currentTask->peekSignal()) {
                        return -SYS_EINTR;
                    }
                }
            } else {
                task::freeTask(pid, stat_loc);
                return pid;
            }
        }
    }
}

}  // namespace nyan::syscall
