#pragma once

#include <stdint.h>
#include <sys/types.h>

#include "forward.hpp"

namespace nyan::task {

extern TaskControlBlock* __all_tasks[__max_task];

pid_t allocPid(TaskControlBlock* task);
void setupKnownTasks();
TaskControlBlock* findTask(pid_t pid);

}  // namespace nyan::task
