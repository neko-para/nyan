#include <nyan/syscall.h>

#include "../task/pid.hpp"
#include "../task/scheduler.hpp"
#include "../task/tcb.hpp"

namespace nyan::syscall {

pid_t getpgid(pid_t pid) {
    task::TaskControlBlock* tcb;
    if (pid) {
        tcb = task::findTask(pid);
        if (!tcb) {
            return SYS_ESRCH;
        }
    } else {
        tcb = task::__scheduler->__current;
    }
    return tcb->groupPid;
}

}  // namespace nyan::syscall
