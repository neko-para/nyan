#pragma once

#include <stdint.h>

namespace nyan::task {

using pid_t = int32_t;

enum KnownPid : pid_t {
    KP_Invalid = -1,

    KP_Idle = 1,
    KP_Init = 15,

    KP_FirstUser = 16,
};

struct TaskControlBlock;

constexpr int32_t MaxTaskCount = 256;

extern TaskControlBlock* allTasks[MaxTaskCount];

pid_t allocPid(TaskControlBlock* task);
void setupKnownTasks();

}  // namespace nyan::task
