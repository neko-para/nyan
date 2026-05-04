#include <nyan/syscall.h>

#include "../task/scheduler.hpp"
#include "../task/tcb.hpp"

namespace nyan::syscall {

int set_tid_address(int*) {
    return task::__scheduler->__current->pid;
}

}  // namespace nyan::syscall
