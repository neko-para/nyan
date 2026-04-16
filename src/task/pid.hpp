#pragma once

#include <sys/types.h>

namespace nyan::task {

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
TaskControlBlock* findTask(pid_t pid);

}  // namespace nyan::task
