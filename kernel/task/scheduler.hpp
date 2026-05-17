#pragma once

#include <span>

#include "../interrupt/forward.hpp"
#include "../lib/list.hpp"
#include "forward.hpp"

namespace nyan::task {

struct Scheduler {
    TaskControlBlock* __current{};
    uint32_t __disable_yield_counter{};
    bool __need_yield{};
    lib::List<TaskControlBlockTag, true> __pending;
    lib::List<TaskControlBlockTag, true> __sleeping;

    // task
    pid_t addTask(TaskControlBlock* task) noexcept;
    bool freeTask(pid_t pid, int* stat) noexcept;
    void execTask(std::span<uint8_t> file,
                  std::vector<std::string> argv,
                  std::vector<std::string> env,
                  interrupt::SyscallFrame* frame) noexcept;

    // self
    [[noreturn]] void exit(int code, int sig = 0) noexcept;
    void yield() noexcept;
    void disableYield() noexcept { __disable_yield_counter++; }
    void enableYield() noexcept;
    bool yieldEnabled() const noexcept { return __disable_yield_counter == 0; }

    WakeReason block(BlockReason reason) noexcept;
    WakeReason sleep(uint64_t ms, uint64_t* rest) noexcept;
    bool checkSignal(interrupt::SyscallFrame* frame) noexcept;

    bool isInterrupted() const noexcept;

    // other
    void wake(TaskControlBlock* task, WakeReason reason) noexcept;
    void checkSleep() noexcept;
    void sendSignal(TaskControlBlock* task, int sig) noexcept;
    void raise(int sig) noexcept { sendSignal(__current, sig); }
};

}  // namespace nyan::task