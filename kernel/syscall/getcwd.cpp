#include <nyan/syscall.h>

#include "../fs/dentry.hpp"
#include "utils.hpp"

namespace nyan::syscall {

char* getcwd(char* buf, size_t size) {
    if (size == 0) {
        return reinterpret_cast<char*>(-SYS_EINVAL);
    }
    if (!utils::validateWrite(buf, size)) {
        return reinterpret_cast<char*>(-SYS_EFAULT);
    }
    auto cwd = utils::getCwd()->asPath();
    if (size < cwd.size() + 1) {
        return reinterpret_cast<char*>(-SYS_ERANGE);
    }
    memcpy(buf, cwd.c_str(), cwd.size() + 1);
    return buf;
}

}  // namespace nyan::syscall
