#include <nyan/syscall.h>

#include "../task/scheduler.hpp"
#include "../task/tcb.hpp"

namespace nyan::syscall {

pid_t getpgrp() {
    return task::__scheduler->__current->groupPid;
}

}  // namespace nyan::syscall
