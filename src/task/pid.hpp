#pragma once

#include <sys/types.h>

#include "forward.hpp"

namespace nyan::task {

struct TaskControlBlock;

constexpr int32_t MaxTaskCount = 256;

extern TaskControlBlock* allTasks[MaxTaskCount];

pid_t allocPid(TaskControlBlock* task);
void setupKnownTasks();
TaskControlBlock* findTask(pid_t pid);

}  // namespace nyan::task
