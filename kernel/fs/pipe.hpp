#pragma once

#include <optional>

#include "../arch/guard.hpp"
#include "../task/wait.hpp"
#include "file.hpp"

namespace nyan::fs {

constexpr size_t PipeBufferSize = 4096;

struct PipeObj : public FileObj {
    char* __buffer;
    size_t __head{};
    size_t __tail{};
    task::WaitList __read_wait;
    task::WaitList __write_wait;
    bool __read_alive{true};
    bool __write_alive{true};

    PipeObj();
    virtual ~PipeObj();

    size_t size() const noexcept { return __head - __tail; }
    bool empty() const noexcept { return __head == __tail; }
    bool full() const noexcept { return size() == PipeBufferSize; }

    std::optional<arch::InterruptGuard> syncWaitForRead() noexcept;
    std::optional<arch::InterruptGuard> syncWaitForWrite() noexcept;

    virtual ssize_t read(void* buf, size_t size) noexcept override;
    virtual ssize_t write(const void* buf, size_t size) noexcept override;
    virtual int ioctl(uint32_t req, uint32_t param) noexcept override;

    virtual void onFdClose(uint32_t mode) noexcept override;
};

}  // namespace nyan::fs
