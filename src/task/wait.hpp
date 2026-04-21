#pragma once

#include "../lib/list.hpp"
#include "forward.hpp"

namespace nyan::task {

struct TaskControlBlockTag;

struct WaitList {
    lib::TailList<TaskControlBlockTag> list;

    bool empty() noexcept { return !list.head; }

    WakeReason wait(BlockReason reason) noexcept;
    bool wakeOne(WakeReason reason) noexcept;
    void wakeAll(WakeReason reason) noexcept;
};

}  // namespace nyan::task
