#include <nyan/syscall.h>

#include "../task/mod.hpp"

namespace nyan::syscall {

int kill(pid_t pid, int sig) {
    return task::kill(pid, sig).merge();
}

}  // namespace nyan::syscall
