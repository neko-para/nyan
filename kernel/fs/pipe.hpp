#pragma once

#include <optional>

#include "../arch/guard.hpp"
#include "../task/wait.hpp"
#include "file.hpp"

namespace nyan::fs {

constexpr size_t PipeBufferSize = 4096;

struct PipeState : public lib::Shared {
    char* __buffer;
    size_t __head{};
    size_t __tail{};
    task::WaitList __read_wait;
    task::WaitList __write_wait;
    bool __read_alive{true};
    bool __write_alive{true};

    PipeState();
    virtual ~PipeState() override;

    size_t size() const noexcept { return __head - __tail; }
    bool empty() const noexcept { return __head == __tail; }
    bool full() const noexcept { return size() == PipeBufferSize; }

    std::optional<arch::InterruptGuard> syncWaitForRead() noexcept;
    std::optional<arch::InterruptGuard> syncWaitForWrite() noexcept;

    void onReadClosed() noexcept;
    void onWriteClosed() noexcept;

    ssize_t read(void* buf, size_t size) noexcept;
    ssize_t write(const void* buf, size_t size) noexcept;
};

struct PipeObj : public FileObj {
    lib::Ref<PipeState> __state;

    PipeObj(lib::Ref<PipeState> state, uint32_t mode) noexcept : FileObj(mode), __state(state) {}

    virtual ssize_t read(void* buf, size_t size) noexcept override;
    virtual ssize_t write(const void* buf, size_t size) noexcept override;
    virtual int ioctl(uint32_t req, uint32_t param) noexcept override;

    virtual void onFdClose() noexcept override;
};

}  // namespace nyan::fs
