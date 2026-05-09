#include <nyan/syscall.h>

#include "../arch/print.hpp"
#include "../fs/dentry.hpp"
#include "../fs/mod.hpp"
#include "../fs/vnode.hpp"
#include "../task/mod.hpp"
#include "../task/scheduler.hpp"

namespace nyan::syscall {

int execve(const char* __pathname, char* const __argv[], char* const __envp[], interrupt::SyscallFrame* frame) {
    auto pathname = __try(task::checkString(__pathname));
    auto argv = __try(task::checkArgv(__argv));
    auto envp = __try(task::checkArgv(__envp));

    auto dentry = __try(fs::resolve(pathname));
    auto vnode = dentry->effectiveVNode();

    struct stat info;
    __try
        (vnode->stat(&info));

    // TODO check permission
    if (!(info.st_mode & S_IFREG)) {
        return SYS_EACCES;
    }

    std::unique_ptr<uint8_t[]> file(new uint8_t[info.st_size]);
    __try
        (vnode->read(file.get(), info.st_size, 0));

    arch::kprint("exec {}\n", pathname);
    task::__scheduler->execTask(std::span{file.get(), static_cast<size_t>(info.st_size)}, std::move(argv),
                                std::move(envp), frame);
    return 0;
}

}  // namespace nyan::syscall
