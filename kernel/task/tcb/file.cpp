#include "file.hpp"

#include <nyan/errno.h>

namespace nyan::task {

Result<lib::Ref<fs::FdObj>> TaskFileInfo::getFile(int fd) noexcept {
    auto slot = getFileSlot(fd);
    if (!slot) {
        return slot.error();
    }
    if (!*slot) {
        return SYS_EBADF;
    }
    return **slot;
}

Result<lib::Ref<fs::FdObj>*> TaskFileInfo::getFileSlot(int fd) noexcept {
    if (fd < 0 || static_cast<size_t>(fd) >= __max_fd) {
        return SYS_EBADF;
    }
    return &__fd_table[fd];
}

Result<lib::Ref<fs::FdObj>*> TaskFileInfo::findFileSlot(int& fd, int hint) noexcept {
    for (size_t i = hint; i < __max_fd; i++) {
        if (__fd_table[i]) {
            continue;
        }
        fd = i;
        return &__fd_table[i];
    }
    return SYS_EMFILE;
}

void TaskFileInfo::prepareForExec() noexcept {
    for (auto& fd : __fd_table) {
        if (fd && fd->__close_on_exec) {
            fd = {};
        }
    }
}

}  // namespace nyan::task
