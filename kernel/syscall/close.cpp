#include <nyan/syscall.h>

#include "../task/mod.hpp"

namespace nyan::syscall {

int close(int fd) {
    __try
        (task::closeFd(fd));
    return 0;
}

}  // namespace nyan::syscall
