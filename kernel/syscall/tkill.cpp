#include <nyan/syscall.h>

namespace nyan::syscall {

int tkill(pid_t pid, int sig) {
    return kill(pid, sig);
}

}  // namespace nyan::syscall
