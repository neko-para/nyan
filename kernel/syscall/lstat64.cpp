#include <nyan/syscall.h>

#include "../fs/dentry.hpp"
#include "../fs/mod.hpp"
#include "../fs/vnode.hpp"
#include "../task/mod.hpp"

namespace nyan::syscall {

int lstat64(const char* __filename, struct stat* statbuf) {
    auto filename = __try(task::checkString(__filename));
    __try
        (task::checkW(statbuf));

    auto dentry = __try(fs::resolve({filename}, {false}));
    __try
        (dentry->effectiveVNode()->stat(statbuf));

    return 0;
}

}  // namespace nyan::syscall
