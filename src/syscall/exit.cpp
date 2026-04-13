#include "entry.hpp"

#include "../task/task.hpp"

namespace nyan::syscall {

void exit(int code) {
    task::exitTask(code);
}

}  // namespace nyan::syscall
