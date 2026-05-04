#include <nyan/syscall.h>

#include "../fs/dentry.hpp"
#include "../task/scheduler.hpp"
#include "../task/tcb.hpp"

namespace nyan::syscall {

char* getcwd(char* buf, size_t size) {
    if (!buf) {
        return reinterpret_cast<char*>(-SYS_EFAULT);
    }
    if (size == 0) {
        return reinterpret_cast<char*>(-SYS_EINVAL);
    }
    auto cwd = task::__scheduler->__current->cwd->asPath();
    if (size < cwd.size() + 1) {
        return reinterpret_cast<char*>(-SYS_ERANGE);
    }
    memcpy(buf, cwd.c_str(), cwd.size() + 1);
    return buf;
}

}  // namespace nyan::syscall
