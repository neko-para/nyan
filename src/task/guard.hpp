#pragma once

#include "../arch/io.hpp"

namespace nyan::task {

struct InterruptGuard {
    uint32_t flags;

    InterruptGuard() {
        flags = arch::flags();
        arch::cli();
    }
    ~InterruptGuard() { arch::setFlags(flags); }
    InterruptGuard(const InterruptGuard&) = delete;
    InterruptGuard& operator=(const InterruptGuard&) = delete;
};

}  // namespace nyan::task
