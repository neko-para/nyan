#include <nyan/syscall.h>

#include <string_view>

#include "../arch/print.hpp"
#include "../fs/dentry.hpp"
#include "../fs/vnode.hpp"
#include "../task/scheduler.hpp"

namespace nyan::syscall {

int execve(const char* pathname, char* const argv[], char* const envp[], interrupt::SyscallFrame* frame) {
    // TODO: resolve via PATH

    auto dentry = fs::resolve(pathname);
    if (!dentry || !dentry->__node) {
        return -SYS_ENOENT;
    }

    struct stat info;
    if (dentry->__node->stat(&info)) {
        return -SYS_EACCES;
    }

    // TODO check permission
    if (!(info.st_mode & S_IFREG)) {
        return -SYS_EACCES;
    }

    std::unique_ptr<uint8_t[]> file(new uint8_t[info.st_size]);
    auto ret = dentry->__node->read(file.get(), info.st_size, 0);
    if (ret < 0) {
        return ret;
    }

    std::vector<std::string> args;
    std::vector<std::string> envs;

    for (auto ptr = argv; *ptr; ptr++) {
        args.push_back(*ptr);
    }

    for (auto ptr = envp; *ptr; ptr++) {
        envs.push_back(*ptr);
    }

    arch::kprint("exec {}\n", pathname);
    task::__scheduler->execTask(std::span{file.get(), static_cast<size_t>(info.st_size)}, std::move(args),
                                std::move(envs), frame);
    return 0;
}

}  // namespace nyan::syscall
