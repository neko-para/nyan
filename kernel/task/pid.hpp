#pragma once

#include <stdint.h>
#include <sys/types.h>
#include <vector>

#include "../lib/result.hpp"
#include "forward.hpp"

namespace nyan::task {

extern TaskControlBlock* __all_tasks[__max_task];

pid_t allocPid(TaskControlBlock* task);
void setupKnownTasks();
TaskControlBlock* findTask(pid_t pid) noexcept;
Result<TaskControlBlock*> findTaskNew(pid_t pid) noexcept;
Result<std::vector<TaskControlBlock*>> findTaskGroup(pid_t pgid) noexcept;

}  // namespace nyan::task
