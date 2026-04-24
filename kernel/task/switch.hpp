#pragma once

#include <sys/types.h>

namespace nyan::task {

struct TaskControlBlock;

extern "C" void switchToTask(TaskControlBlock* nextTask);
extern "C" [[noreturn]] void jumpRing3(uint32_t entry, uint32_t esp);
extern "C" [[noreturn]] void syscallReturn(void* esp);

}  // namespace nyan::task
