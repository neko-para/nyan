#include <nyan/syscall.h>

#include "../task/scheduler.hpp"

namespace nyan::syscall {

void exit(int error_code) {
    task::__scheduler->exit(error_code);
}

}  // namespace nyan::syscall
