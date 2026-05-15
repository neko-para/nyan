#pragma once

#include "../arch/guard.hpp"
#include "../task/wait.hpp"
#include "file.hpp"

namespace nyan::fs {

constexpr size_t __pipe_buffer_size = 0x1000;
constexpr size_t __pipe_buffer_mask = 0xFFF;

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
    bool full() const noexcept { return size() == __pipe_buffer_size; }

    Result<arch::InterruptGuard> syncWaitForRead() noexcept;
    Result<arch::InterruptGuard> syncWaitForWrite() noexcept;

    void onReadClosed() noexcept;
    void onWriteClosed() noexcept;

    Result<ssize_t> read(void* buf, size_t size) noexcept;
    Result<ssize_t> write(const void* buf, size_t size) noexcept;
};

struct PipeObj : public FileObj {
    lib::Ref<PipeState> __state;

    PipeObj(lib::Ref<PipeState> state, uint32_t mode) noexcept : FileObj(mode), __state(state) {}

    virtual Result<ssize_t> read(void* buf, size_t size) noexcept override;
    virtual Result<ssize_t> write(const void* buf, size_t size) noexcept override;
    virtual Result<> stat(struct stat* buf) noexcept override;
    virtual Result<> ioctl(uint32_t req, uint32_t param) noexcept override;
    virtual Result<off_t> seek(off_t offset, int whence) noexcept override;

    virtual void onFdClose() noexcept override;
};

}  // namespace nyan::fs
