#pragma once

#include "../lib/list.hpp"
#include "forward.hpp"

namespace nyan::task {

struct TaskControlBlockTag;

struct WaitList {
    lib::TailList<TaskControlBlockTag> list;

    bool empty() noexcept { return !list.head; }

    // TODO: 被信号唤醒需要返回
    void wait(BlockReason reason) noexcept;
    bool wakeOne() noexcept;
    void wakeAll() noexcept;
};

}  // namespace nyan::task
