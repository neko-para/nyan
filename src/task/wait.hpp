#pragma once

#include "../lib/list.hpp"
#include "forward.hpp"

namespace nyan::task {

struct TaskControlBlockTag;

struct WaitList {
    lib::List<TaskControlBlockTag, true> list;

    bool empty() noexcept { return list.empty(); }

    WakeReason wait(BlockReason reason) noexcept;
    bool wakeOne(WakeReason reason) noexcept;
    void wakeAll(WakeReason reason) noexcept;

    void take(TaskControlBlock* tcb) noexcept;
};

}  // namespace nyan::task
