#pragma once

#include <optional>
#include <string>
#include <vector>

namespace nyan::syscall::utils {

bool validateRead(const void* ptr, size_t size) noexcept;
bool validateWrite(void* ptr, size_t size) noexcept;
bool validateReadWrite(void* ptr, size_t size) noexcept;
bool validateExec(void* ptr) noexcept;

std::optional<std::string> validateString(const char* str) noexcept;
std::optional<std::vector<std::string>> validateStringArray(const char* const* strs) noexcept;

template <typename T>
inline bool validateReadAuto(const T* ptr, size_t count = 1, bool nullable = false) noexcept {
    if (nullable && !ptr) {
        return true;
    }
    return validateRead(ptr, sizeof(T) * count);
}
template <typename T>
inline bool validateWriteAuto(T* ptr, size_t count = 1, bool nullable = false) noexcept {
    if (nullable && !ptr) {
        return true;
    }
    return validateWrite(ptr, sizeof(T) * count);
}
template <typename T>
inline bool validateReadWriteAuto(T* ptr, size_t count = 1, bool nullable = false) noexcept {
    if (nullable && !ptr) {
        return true;
    }
    return validateReadWrite(ptr, sizeof(T) * count);
}
template <typename T>
inline bool validateExec(T ptr) noexcept {
    return validateExec(reinterpret_cast<void*>(ptr));
}

}  // namespace nyan::syscall::utils
