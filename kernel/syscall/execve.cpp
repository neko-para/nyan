#include <nyan/syscall.h>

#include <string_view>

#include "../arch/print.hpp"
#include "../data/embed.hpp"
#include "../fs/load.hpp"
#include "../fs/vnode.hpp"
#include "../task/task.hpp"

namespace nyan::syscall {

int execve(const char* pathname,
           char* const argv[],
           [[maybe_unused]] char* const envp[],
           interrupt::SyscallFrame* frame) {
    // TODO: resolve via PATH

    auto vnode = fs::resolve(pathname);
    if (!vnode) {
        return -SYS_ENOENT;
    }

    struct stat info;
    if (vnode->stat(&info)) {
        return -SYS_EACCES;
    }

    // TODO check permission
    if (!(info.st_mode & S_IFREG)) {
        return -SYS_EACCES;
    }

    std::unique_ptr<uint8_t[]> file(new uint8_t[info.st_size]);
    auto ret = vnode->read(file.get(), info.st_size, 0);
    if (ret < 0) {
        return ret;
    }

    arch::kprint("exec {}\n", pathname);
    task::execTask(file.get(), info.st_size, argv, frame);
    return 0;
}

}  // namespace nyan::syscall
