#include "print.hpp"

#include "../task/tcb.hpp"

namespace nyan::arch {

pid_t __safe_getpid() {
    return task::currentTask ? task::currentTask->pid : 0;
}

}  // namespace nyan::arch
