#include <nyan/syscall.h>

#include "../task/scheduler.hpp"
#include "../task/tcb.hpp"

namespace nyan::syscall {

pid_t gettid() {
    return task::__scheduler->__current->pid;
}

}  // namespace nyan::syscall
