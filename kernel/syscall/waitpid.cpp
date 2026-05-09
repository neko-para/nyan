#include <nyan/syscall.h>

#include <sys/wait.h>

#include "../task/mod.hpp"

namespace nyan::syscall {

pid_t waitpid(pid_t pid, int* stat_loc, int options) {
    __try
        (task::checkW(stat_loc, 1, true));

    auto [resultPid, stat] = __try(task::waitpid(pid, options));
    if (stat_loc) {
        *stat_loc = stat;
    }
    return resultPid;
}

}  // namespace nyan::syscall
