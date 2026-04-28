#include <nyan/syscall.h>

#include "../task/tcb.hpp"

namespace nyan::syscall {

pid_t gettid() {
    return task::currentTask->pid;
}

}  // namespace nyan::syscall
