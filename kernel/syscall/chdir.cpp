#include <nyan/syscall.h>

#include "../fs/mod.hpp"
#include "../task/mod.hpp"

namespace nyan::syscall {

int chdir(const char* __pathname) {
    auto pathname = __try(task::checkString(__pathname));
    auto dentry = __try(fs::resolve(pathname));
    __try
        (task::setCwd(dentry));
    return 0;
}

}  // namespace nyan::syscall
