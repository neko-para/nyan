#include "file.hpp"

#include <nyan/errno.h>
#include <string_view>

#include "../arch/print.hpp"

namespace nyan::arch {

/*
ssize_t DebugConObj::read(void* buf, size_t size) noexcept {
    std::ignore = buf;
    std::ignore = size;
    return -SYS_EBADF;
}

ssize_t DebugConObj::write(const void* buf, size_t size) noexcept {
    arch::kprint("{}", std::string_view{static_cast<const char*>(buf), size});
    return size;
}

int DebugConObj::ioctl(uint32_t req, uint32_t param) noexcept {
    std::ignore = req;
    std::ignore = param;
    return -SYS_ENOTTY;
}
*/

}  // namespace nyan::arch
