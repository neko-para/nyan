#include "file.hpp"

namespace nyan::task {

lib::Ref<fs::FdObj>* TaskFileInfo::getFile(int fd) noexcept {
    if (fd < 0 || static_cast<size_t>(fd) >= __max_fd) {
        return nullptr;
    }
    return &__fd_table[fd];
}

lib::Ref<fs::FdObj>* TaskFileInfo::getFileSlot(int& fd, int hint) noexcept {
    for (size_t i = hint; i < __max_fd; i++) {
        if (__fd_table[i]) {
            continue;
        }
        fd = i;
        return &__fd_table[i];
    }
    return nullptr;
}

}  // namespace nyan::task