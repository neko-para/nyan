#include <nyan/syscall.h>

#include "../fs/dentry.hpp"
#include "../task/mod.hpp"

namespace nyan::syscall {

char* getcwd(char* buf, size_t size) {
    if (size == 0) {
        return SYS_EINVAL;
    }

    __try
        (task::checkW(buf, size));

    auto cwd = task::getCwd()->asPath();
    if (size < cwd.size() + 1) {
        return SYS_ERANGE;
    }
    memcpy(buf, cwd.c_str(), cwd.size() + 1);
    return buf;
}

}  // namespace nyan::syscall
