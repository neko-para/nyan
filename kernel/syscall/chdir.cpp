#include <nyan/syscall.h>

#include "../fs/mod.hpp"
#include "../task/mod.hpp"
#include "utils.hpp"

namespace nyan::syscall {

int chdir(const char* pathname) {
    auto path = utils::validateString(pathname);
    if (!path) {
        return SYS_EFAULT;
    }
    auto dentry = __try(fs::resolve(*path));
    __try
        (task::setCwd(dentry));
    return 0;
}

}  // namespace nyan::syscall
