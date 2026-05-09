#include <nyan/syscall.h>

#include "../task/scheduler.hpp"
#include "../task/tcb.hpp"

namespace nyan::syscall {

pid_t getppid() {
    return task::__scheduler->__current->parentPid;
}

}  // namespace nyan::syscall
