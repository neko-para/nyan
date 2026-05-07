#include "utils.hpp"

#include "../task/scheduler.hpp"
#include "../task/tcb.hpp"

namespace nyan::syscall::utils {

static auto current() {
    return task::__scheduler->__current;
}

bool validateRead(const void* ptr, size_t size) noexcept {
    return current()->vmSpace.empty() || current()->vmSpace.validate(paging::VirtualAddress{ptr}, size, PROT_READ);
}

bool validateWrite(void* ptr, size_t size) noexcept {
    return current()->vmSpace.empty() || current()->vmSpace.validate(paging::VirtualAddress{ptr}, size, PROT_WRITE);
}

bool validateReadWrite(void* ptr, size_t size) noexcept {
    return current()->vmSpace.empty() ||
           current()->vmSpace.validate(paging::VirtualAddress{ptr}, size, PROT_READ | PROT_WRITE);
}

bool validateExec(void* ptr) noexcept {
    return current()->vmSpace.empty() || current()->vmSpace.validate(paging::VirtualAddress{ptr}, 1, PROT_EXEC);
}

std::optional<std::string> validateString(const char* str) noexcept {
    auto result = current()->vmSpace.validateReadonlyRange<const char>(paging::VirtualAddress{str});
    if (!result) {
        return std::nullopt;
    }
    return std::string{result->begin(), result->end()};
}

std::optional<std::vector<std::string>> validateStringArray(const char* const* strs) noexcept {
    auto ptrs = current()->vmSpace.validateReadonlyRange<const char* const>(paging::VirtualAddress{strs});
    if (!ptrs) {
        return std::nullopt;
    }
    std::vector<std::string> result;
    for (auto ptr : *ptrs) {
        auto str = current()->vmSpace.validateReadonlyString(paging::VirtualAddress{ptr});
        if (!str) {
            return std::nullopt;
        }
        result.push_back(std::move(*str));
    }
    return result;
}

lib::Ref<fs::DEntry> getCwd() noexcept {
    return current()->cwd;
}

void setCwd(lib::Ref<fs::DEntry> dir) noexcept {
    current()->cwd = dir;
}

}  // namespace nyan::syscall::utils
