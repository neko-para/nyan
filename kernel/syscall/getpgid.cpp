#include <nyan/syscall.h>

#include "../task/pid.hpp"
#include "../task/tcb.hpp"

namespace nyan::syscall {

pid_t getpgid(pid_t pid) {
    auto tcb = task::findTask(pid);
    if (!tcb) {
        return SYS_ESRCH;
    }
    return tcb->groupPid;
}

}  // namespace nyan::syscall
