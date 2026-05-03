#pragma once

#include "../lib/list.hpp"
#include "forward.hpp"

namespace nyan::task {

struct WaitList {
    lib::List<TaskControlBlockTag, true> __list;

    bool empty() noexcept { return __list.empty(); }

    WakeReason wait(BlockReason reason) noexcept;
    bool wakeOne(WakeReason reason) noexcept;
    void wakeAll(WakeReason reason) noexcept;
};

}  // namespace nyan::task
