#pragma once

#include "../arch/instr.hpp"

namespace nyan::task {

struct InterruptGuard {
    uint32_t flags;

    InterruptGuard() {
        flags = arch::flags();
        arch::cli();
    }
    ~InterruptGuard() {
        if (flags) {
            arch::setFlags(flags);
        }
    }
    InterruptGuard(const InterruptGuard&) = delete;
    InterruptGuard& operator=(const InterruptGuard&) = delete;
    InterruptGuard(InterruptGuard&& guard) {
        flags = guard.flags;
        guard.flags = 0;
    }
    InterruptGuard& operator=(InterruptGuard&& guard) {
        if (this == &guard) {
            return *this;
        }
        if (flags) {
            arch::setFlags(flags);
        }
        flags = guard.flags;
        guard.flags = 0;
        return *this;
    }
};

}  // namespace nyan::task
