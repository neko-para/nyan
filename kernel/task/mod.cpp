#include "mod.hpp"

#include <nyan/errno.h>

#include "forward.hpp"
#include "scheduler.hpp"
#include "tcb.hpp"

namespace nyan::task {

Result<> closeFd(int fd) noexcept {
    auto fdObjPtr = __try(task::__scheduler->__current->__file.findFileSlot(fd));
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
    auto fdObjPtr = __try(task::__scheduler->__current->__file.findFileSlot(fd));
    *fdObjPtr = lib::makeRef<fs::FdObj>(file);
    return {fd, *fdObjPtr};
}

Result<lib::Ref<fs::FdObj>> installFileTo(lib::Ref<fs::FileObj> file, int fd) noexcept {
    auto fdObjPtr = __try(task::__scheduler->__current->__file.getFileSlot(fd));
    *fdObjPtr = lib::makeRef<fs::FdObj>(file);
    return *fdObjPtr;
}

Result<int> installFd(lib::Ref<fs::FdObj> fdobj) noexcept {
    int fd;
    auto fdObjPtr = __try(task::__scheduler->__current->__file.findFileSlot(fd));
    *fdObjPtr = fdobj;
    return fd;
}

Result<> installFdTo(lib::Ref<fs::FdObj> fdobj, int fd) noexcept {
    auto fdObjPtr = __try(task::__scheduler->__current->__file.getFileSlot(fd));
    *fdObjPtr = fdobj;
    return {};
}

}  // namespace nyan::task
