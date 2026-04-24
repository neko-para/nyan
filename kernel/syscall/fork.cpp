#include <nyan/syscall.h>

#include "../task/task.hpp"

namespace nyan::syscall {

pid_t fork(interrupt::SyscallFrame* frame) {
    return task::forkTask(frame);
}

}  // namespace nyan::syscall
