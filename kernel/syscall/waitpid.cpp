#include <nyan/syscall.h>

#include <sys/wait.h>

#include "../task/mod.hpp"

namespace nyan::syscall {

pid_t waitpid(pid_t pid, int* stat_addr, int options) {
    __try
        (task::checkW(stat_addr, 1, true));

    auto [resultPid, stat] = __try(task::waitpid(pid, options));
    if (stat_addr) {
        *stat_addr = stat;
    }
    return resultPid;
}

}  // namespace nyan::syscall
