#include <nyan/syscall.h>

#include "../task/tcb.hpp"

namespace nyan::syscall {

char* getcwd(char* buf, size_t size) {
    if (!buf) {
        return reinterpret_cast<char*>(-SYS_EFAULT);
    }
    if (size == 0) {
        return reinterpret_cast<char*>(-SYS_EINVAL);
    }
    if (size < task::currentTask->cwd.size() + 1) {
        return reinterpret_cast<char*>(-SYS_ERANGE);
    }
    memcpy(buf, task::currentTask->cwd.c_str(), task::currentTask->cwd.size() + 1);
    return buf;
}

}  // namespace nyan::syscall
