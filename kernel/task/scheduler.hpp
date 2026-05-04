#pragma once

#include "../lib/list.hpp"
#include "forward.hpp"

namespace nyan::task {

struct Scheduler {
    TaskControlBlock* __current{};
    lib::List<TaskControlBlockTag, true> __pending;
    lib::List<TaskControlBlockTag, true> __sleeping;
};

}  // namespace nyan::task