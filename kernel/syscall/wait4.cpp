#include <nyan/syscall.h>

namespace nyan::syscall {

pid_t wait4(pid_t pid, int* stat_addr, int options, struct rusage*) {
    return waitpid(pid, stat_addr, options);
}

}  // namespace nyan::syscall
