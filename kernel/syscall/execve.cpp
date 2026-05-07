#include <nyan/syscall.h>

#include <string_view>

#include "../arch/print.hpp"
#include "../fs/dentry.hpp"
#include "../fs/vnode.hpp"
#include "../task/scheduler.hpp"
#include "utils.hpp"

namespace nyan::syscall {

int execve(const char* pathname, char* const argv[], char* const envp[], interrupt::SyscallFrame* frame) {
    auto args = utils::validateStringArray(argv);
    auto envs = utils::validateStringArray(envp);
    if (!args || !envs) {
        return SYS_EFAULT;
    }

    auto [dentry, _1, _2] = fs::resolve(pathname);
    if (!dentry || !dentry->__node) {
        return SYS_ENOENT;
    }

    struct stat info;
    __try
        (dentry->__node->stat(&info));

    // TODO check permission
    if (!(info.st_mode & S_IFREG)) {
        return SYS_EACCES;
    }

    std::unique_ptr<uint8_t[]> file(new uint8_t[info.st_size]);
    __try
        (dentry->__node->read(file.get(), info.st_size, 0));

    arch::kprint("exec {}\n", pathname);
    task::__scheduler->execTask(std::span{file.get(), static_cast<size_t>(info.st_size)}, std::move(*args),
                                std::move(*envs), frame);
    return 0;
}

}  // namespace nyan::syscall
