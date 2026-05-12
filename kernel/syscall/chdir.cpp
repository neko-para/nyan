#include <nyan/syscall.h>

#include "../fs/mod.hpp"
#include "../task/mod.hpp"

namespace nyan::syscall {

int chdir(const char* __filename) {
    auto filename = __try(task::checkString(__filename));
    auto dentry = __try(fs::resolve(filename));
    __try
        (task::setCwd(dentry));
    return 0;
}

}  // namespace nyan::syscall
