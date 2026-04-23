#include <nyan/syscall.h>

#include "../task/task.hpp"
#include "../task/tcb.hpp"

namespace nyan::syscall {

int getpid() {
    return task::currentTask->pid;
}

}  // namespace nyan::syscall
