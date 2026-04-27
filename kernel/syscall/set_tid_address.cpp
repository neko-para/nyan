#include <nyan/syscall.h>

#include "../task/tcb.hpp"

namespace nyan::syscall {

int set_tid_address(int*) {
    return task::currentTask->pid;
}

}  // namespace nyan::syscall
