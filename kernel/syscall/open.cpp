#include <fcntl.h>
#include <nyan/syscall.h>

#include "../arch/print.hpp"
#include "../fs/fd.hpp"
#include "../fs/mod.hpp"
#include "../task/mod.hpp"

namespace nyan::syscall {

int open(const char* __filename, int flags, mode_t mode) {
    auto filename = __try(task::checkString(__filename));

    arch::kprint("open {}", filename);

    auto file = __try(fs::open(filename, flags, mode));

    auto [fd, fdObj] = __try(task::installFile(file));
    if (flags & O_CLOEXEC) {
        fdObj->__close_on_exec = true;
    }
    return fd;
}

}  // namespace nyan::syscall
