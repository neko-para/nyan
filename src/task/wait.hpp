#pragma once

#include "task.hpp"

namespace nyan::task {

struct WaitList {
    lib::TailList<TaskControlBlock> list;

    bool empty() noexcept { return !list.head; }

    void wait() noexcept;
    bool wakeOne() noexcept;
    void wakeAll() noexcept;
};

}  // namespace nyan::task
