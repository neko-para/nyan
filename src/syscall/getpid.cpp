#include "entry.hpp"

#include "../task/task.hpp"

namespace nyan::syscall {

int getpid() {
    return task::currentTask->pid;
}

}  // namespace nyan::syscall
