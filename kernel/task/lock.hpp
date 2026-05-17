#pragma once

#include <atomic>
#include <mutex>

#include "../arch/instr.hpp"

namespace nyan::task {

struct SpinLockRaw {
    std::atomic<bool> __value;

    void lock() noexcept;
    void unlock() noexcept;
};

struct SpinLock : public SpinLockRaw {};
struct SpinGuard {
    SpinLock* __lock{};

    SpinGuard() noexcept = default;
    SpinGuard(SpinLock& lk) noexcept : __lock(&lk) { __lock->lock(); }
    SpinGuard(SpinLock& lk, std::adopt_lock_t) noexcept : __lock(&lk) {}
    SpinGuard(const SpinGuard&) = delete;
    SpinGuard(SpinGuard&& guard) noexcept : __lock(guard.__lock) { guard.__lock = nullptr; }
    ~SpinGuard() { unlock(); }
    SpinGuard& operator=(const SpinGuard&) = delete;
    SpinGuard& operator=(SpinGuard&& guard) noexcept {
        if (this == &guard) {
            return *this;
        }
        if (__lock) {
            __lock->unlock();
        }
        __lock = guard.__lock;
        guard.__lock = nullptr;
        return *this;
    }

    void unlock() noexcept {
        if (__lock) {
            __lock->unlock();
        }
    }
};

struct IntSpinLock : public SpinLockRaw {};
struct IntSpinGuard {
    IntSpinLock* __lock{};
    uint32_t __eflags{};

    IntSpinGuard() noexcept = default;
    IntSpinGuard(IntSpinLock& lk) noexcept : __lock(&lk) {
        __eflags = arch::flags();
        arch::cli();
        __lock->lock();
    }
    IntSpinGuard(IntSpinLock& lk, uint32_t flags) noexcept : __lock(&lk), __eflags(flags) {}
    IntSpinGuard(const IntSpinGuard&) = delete;
    IntSpinGuard(IntSpinGuard&& guard) noexcept : __lock(guard.__lock), __eflags(guard.__eflags) {
        guard.__lock = nullptr;
        guard.__eflags = 0;
    }
    ~IntSpinGuard() { unlock(); }
    IntSpinGuard& operator=(const IntSpinGuard&) = delete;
    IntSpinGuard& operator=(IntSpinGuard&& guard) noexcept {
        if (this == &guard) {
            return *this;
        }
        if (__lock) {
            __lock->unlock();
        }
        __lock = guard.__lock;
        __eflags = guard.__eflags;
        guard.__lock = nullptr;
        guard.__eflags = 0;
        return *this;
    }

    void unlock() noexcept {
        if (__lock) {
            __lock->unlock();
            arch::setFlags(__eflags);
        }
    }
};

}  // namespace nyan::task