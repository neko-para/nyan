#pragma once

#include <sys/types.h>
#include <tuple>

#include "../fs/forward.hpp"
#include "../lib/result.hpp"
#include "../lib/shared.hpp"

namespace nyan::task {

void load() noexcept;

Result<> closeFd(int fd) noexcept;
Result<lib::Ref<fs::FdObj>> getFd(int fd) noexcept;
lib::Ref<fs::FdObj> makeFd(lib::Ref<fs::FileObj> file) noexcept;
Result<std::tuple<int, lib::Ref<fs::FdObj>>> installFile(lib::Ref<fs::FileObj> file) noexcept;
Result<lib::Ref<fs::FdObj>> installFileTo(lib::Ref<fs::FileObj> file, int fd) noexcept;
Result<int> installFd(lib::Ref<fs::FdObj> fdobj) noexcept;
Result<> installFdTo(lib::Ref<fs::FdObj> fdobj, int fd) noexcept;

lib::Ref<fs::DEntry> getCwd() noexcept;
Result<> setCwd(lib::Ref<fs::DEntry> dentry) noexcept;

Result<std::tuple<pid_t, int>> waitpid(pid_t pid, int options);

}  // namespace nyan::task
