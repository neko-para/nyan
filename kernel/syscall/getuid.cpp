#include <nyan/syscall.h>

namespace nyan::syscall {

uid_t getuid() {
    return 0;
}

gid_t getgid() {
    return 0;
}

uid_t geteuid() {
    return 0;
}

gid_t getegid() {
    return 0;
}

}  // namespace nyan::syscall
