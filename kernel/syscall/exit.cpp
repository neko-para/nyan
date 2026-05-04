#include <nyan/syscall.h>

#include "../task/scheduler.hpp"

namespace nyan::syscall {

void exit(int code) {
    task::__scheduler->exit(code);
}

}  // namespace nyan::syscall
