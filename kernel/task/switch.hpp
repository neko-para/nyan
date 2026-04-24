#pragma once

#include <sys/types.h>

namespace nyan::task {

struct TaskControlBlock;

extern "C" void switchToTask(TaskControlBlock* nextTask);
extern "C" void jumpRing3(uint32_t entry, int argc, uint32_t argv);

}  // namespace nyan::task
