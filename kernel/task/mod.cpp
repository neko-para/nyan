#include "mod.hpp"

#include <nyan/errno.h>

#include "../fs/vnode.hpp"
#include "forward.hpp"
#include "pid.hpp"
#include "scheduler.hpp"
#include "tcb.hpp"

namespace nyan::task {

Result<> closeFd(int fd) noexcept {
    auto fdObjPtr = __try(__scheduler->__current->__file.getFileSlot(fd));
    if (!*fdObjPtr) {
        return SYS_EBADF;
    }
    *fdObjPtr = {};
    return {};
}

Result<lib::Ref<fs::FdObj>> getFd(int fd) noexcept {
    return __scheduler->__current->__file.getFile(fd);
}

lib::Ref<fs::FdObj> makeFd(lib::Ref<fs::FileObj> file) noexcept {
    return lib::makeRef<fs::FdObj>(file);
}

Result<std::tuple<int, lib::Ref<fs::FdObj>>> installFile(lib::Ref<fs::FileObj> file) noexcept {
    int fd;
    auto fdObjPtr = __try(__scheduler->__current->__file.findFileSlot(fd));
    *fdObjPtr = lib::makeRef<fs::FdObj>(file);
    return {fd, *fdObjPtr};
}

Result<lib::Ref<fs::FdObj>> installFileTo(lib::Ref<fs::FileObj> file, int fd) noexcept {
    auto fdObjPtr = __try(__scheduler->__current->__file.getFileSlot(fd));
    *fdObjPtr = lib::makeRef<fs::FdObj>(file);
    return *fdObjPtr;
}

Result<int> installFd(lib::Ref<fs::FdObj> fdobj) noexcept {
    int fd;
    auto fdObjPtr = __try(__scheduler->__current->__file.findFileSlot(fd));
    *fdObjPtr = fdobj;
    return fd;
}

Result<> installFdTo(lib::Ref<fs::FdObj> fdobj, int fd) noexcept {
    auto fdObjPtr = __try(__scheduler->__current->__file.getFileSlot(fd));
    *fdObjPtr = fdobj;
    return {};
}

lib::Ref<fs::DEntry> getCwd() noexcept {
    return __scheduler->__current->cwd;
}

Result<> setCwd(lib::Ref<fs::DEntry> dentry) noexcept {
    if (!dentry->effectiveVNode()->isDirectory()) {
        return SYS_ENOTDIR;
    }
    __scheduler->__current->cwd = dentry;
    return {};
}

Result<std::tuple<pid_t, int>> waitpid(pid_t pid, int options) noexcept {
    if ((options & WNOHANG) != options) {
        return SYS_EINVAL;
    }

    int stat;
    if (pid < -1) {
        // wait group
        return SYS_ENOSYS;
    } else if (pid == -1) {
        // wait any child
        while (true) {
            if (__scheduler->__current->childTasks.empty()) {
                return SYS_ECHILD;
            }
            for (auto& tcb : __scheduler->__current->childTasks) {
                if (tcb.ended()) {
                    __scheduler->freeTask(tcb.pid, &stat);
                    return {tcb.pid, stat};
                }
            }

            if (options & WNOHANG) {
                return {0, 0};
            }

            __scheduler->__current->waitTaskInfo = {pid};
            if (__scheduler->__current->__wait_childs.wait(BlockReason::BR_WaitTask) == WakeReason::WR_Signal) {
                return SYS_EINTR;
            }
        }
    } else if (pid == 0) {
        // wait any child in same group
        return SYS_ENOSYS;
    } else {
        // wait pid
        while (true) {
            auto tcb = findTask(pid);
            if (!tcb || tcb->parentPid != __scheduler->__current->pid) {
                return SYS_ECHILD;
            }

            if (!tcb->ended()) {
                if (options & WNOHANG) {
                    return {0, 0};
                }

                __scheduler->__current->waitTaskInfo = {pid};
                if (__scheduler->__current->__wait_childs.wait(BlockReason::BR_WaitTask) == WakeReason::WR_Signal) {
                    return SYS_EINTR;
                }
            } else {
                __scheduler->freeTask(pid, &stat);
                return {pid, stat};
            }
        }
    }
}

Result<> kill(pid_t pid, int sig) noexcept {
    if (sig < 0 || sig >= NSIG) {
        return SYS_EINVAL;
    }

    if (pid > 0) {
        auto tcb = __try(task::findTaskNew(pid));
        if (sig == 0) {
            return {};
        }
        __scheduler->sendSignal(tcb, sig);
        return {};
    } else if (pid == 0) {
        auto tcbs = __try(task::findTaskGroup(__scheduler->__current->groupPid));
        if (sig == 0) {
            return {};
        }
        for (auto tcb : tcbs) {
            __scheduler->sendSignal(tcb, sig);
        }
        return {};
    } else if (pid == -1) {
        return SYS_ENOSYS;
    } else {
        auto tcbs = __try(task::findTaskGroup(-pid));
        if (sig == 0) {
            return {};
        }
        for (auto tcb : tcbs) {
            __scheduler->sendSignal(tcb, sig);
        }
        return {};
    }
}

Result<> checkAddr(const void* ptr, size_t size, uint32_t prot) noexcept {
    if (!__scheduler->__current->vmSpace.validate(paging::VirtualAddress{ptr}, size, prot)) {
        return SYS_EFAULT;
    }
    return {};
}

Result<std::string> checkString(const char* str) noexcept {
    auto res = __scheduler->__current->vmSpace.validateReadonlyString(paging::VirtualAddress{str});
    if (!res) {
        return SYS_EFAULT;
    }
    return std::move(*res);
}

Result<std::vector<std::string>> checkArgv(char* const argv[]) noexcept {
    auto res = __scheduler->__current->vmSpace.validateReadonlyStringArray(paging::VirtualAddress{argv});
    if (!res) {
        return SYS_EFAULT;
    }
    return std::move(*res);
}

}  // namespace nyan::task
