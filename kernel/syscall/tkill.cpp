#include <nyan/syscall.h>

namespace nyan::syscall {

int tkill(pid_t tid, int sig) {
    return kill(tid, sig);
}

}  // namespace nyan::syscall
