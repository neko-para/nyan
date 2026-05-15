#pragma once

#include <array>

#include "../../console/forward.hpp"
#include "../../fs/fd.hpp"
#include "../forward.hpp"

namespace nyan::task {

struct TaskFileInfo {
    std::array<lib::Ref<fs::FdObj>, __max_fd> __fd_table;
    console::Tty* __ctty{};

    Result<lib::Ref<fs::FdObj>> getFile(int fd) noexcept;
    Result<lib::Ref<fs::FdObj>*> getFileSlot(int fd) noexcept;
    Result<lib::Ref<fs::FdObj>*> findFileSlot(int& fd, int hint = 0) noexcept;

    void prepareForExec() noexcept;
};

}  // namespace nyan::task
