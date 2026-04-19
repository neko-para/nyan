#pragma once

// #include "task.hpp"
// #include "tcb.hpp"
#include "../lib/list.hpp"
#include "forward.hpp"

namespace nyan::task {

struct TaskControlBlockTag;

struct WaitList {
    lib::TailList<TaskControlBlockTag> list;

    bool empty() noexcept { return !list.head; }

    void wait(BlockReason reason) noexcept;
    bool wakeOne() noexcept;
    void wakeAll() noexcept;
};

}  // namespace nyan::task
