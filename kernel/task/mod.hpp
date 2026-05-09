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

#pragma mark - check utils

template <typename T, typename P>
inline T* __to_ptr(P ptr) noexcept {
    if constexpr (std::is_integral_v<P>) {
        return reinterpret_cast<T*>(ptr);
    } else {
        return static_cast<T*>(ptr);
    }
}

template <typename T, typename P>
inline Result<const T*> checkR(P ptr) noexcept {
    auto p = __to_ptr<const T>(ptr);
    __try
        (checkAddr(p, sizeof(T), PROT_READ));
    return p;
}

template <typename T, typename P>
inline Result<std::span<const T>> checkR(P ptr, size_t cnt) noexcept {
    auto p = __to_ptr<const T>(ptr);
    __try
        (checkAddr(p, sizeof(T) * cnt, PROT_READ));
    return {p, cnt};
}

template <typename T, typename P>
inline Result<T*> checkW(P ptr) noexcept {
    auto p = __to_ptr<T>(ptr);
    __try
        (checkAddr(p, sizeof(T), PROT_WRITE));
    return p;
}

template <typename T, typename P>
inline Result<std::span<T>> checkW(P ptr, size_t cnt) noexcept {
    auto p = __to_ptr<T>(ptr);
    __try
        (checkAddr(p, sizeof(T) * cnt, PROT_WRITE));
    return {p, cnt};
}

template <typename T, typename P>
inline Result<T*> checkRW(P ptr) noexcept {
    auto p = __to_ptr<T>(ptr);
    __try
        (checkAddr(p, sizeof(T), PROT_READ | PROT_WRITE));
    return p;
}

template <typename T, typename P>
inline Result<std::span<T>> checkRW(P ptr, size_t cnt) noexcept {
    auto p = __to_ptr<T>(ptr);
    __try
        (checkAddr(p, sizeof(T) * cnt, PROT_READ | PROT_WRITE));
    return {p, cnt};
}

template <typename P>
inline Result<void*> checkE(P ptr) noexcept {
    __try
        (checkAddr(__to_ptr(ptr), 1, PROT_EXEC));
    return __to_ptr(ptr);
}

}  // namespace nyan::task
