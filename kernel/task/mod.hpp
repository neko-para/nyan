#pragma once

#include <sys/mman.h>
#include <sys/types.h>
#include <span>
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

Result<std::tuple<pid_t, int>> waitpid(pid_t pid, int options) noexcept;

Result<> kill(pid_t pid, int sig) noexcept;

Result<> checkAddr(const void* ptr, size_t size, uint32_t prot) noexcept;
Result<std::string> checkString(const char* str) noexcept;
Result<std::vector<std::string>> checkArgv(char* const argv[]) noexcept;

#pragma mark - check utils

template <typename T>
inline constexpr size_t __size = sizeof(T);
template <>
inline constexpr size_t __size<void> = 1;

template <typename T>
inline Result<> checkR(const T* ptr, size_t cnt = 1, bool nullable = false) noexcept {
    if (nullable && !ptr) {
        return {};
    }
    return checkAddr(ptr, __size<T> * cnt, PROT_READ);
}

template <typename T, std::integral I>
inline Result<const T*> checkR(I ptr) noexcept {
    __try
        (checkAddr(reinterpret_cast<void*>(ptr), __size<T>, PROT_READ));
    return reinterpret_cast<const T*>(ptr);
}

template <typename T>
inline Result<> checkW(T* ptr, size_t cnt = 1, bool nullable = false) noexcept {
    if (nullable && !ptr) {
        return {};
    }
    return checkAddr(ptr, __size<T> * cnt, PROT_WRITE);
}

template <typename T, std::integral I>
inline Result<T*> checkW(I ptr) noexcept {
    __try
        (checkAddr(reinterpret_cast<void*>(ptr), __size<T>, PROT_WRITE));
    return reinterpret_cast<T*>(ptr);
}

inline Result<> checkE(void* ptr) noexcept {
    return checkAddr(ptr, 1, PROT_EXEC);
}

template <typename I>
inline Result<> checkE(I ptr) noexcept {
    return checkAddr(reinterpret_cast<void*>(ptr), 1, PROT_EXEC);
}

}  // namespace nyan::task
