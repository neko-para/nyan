#pragma once

#include <sys/types.h>

namespace nyan::task {

struct TaskControlBlock;

extern "C" void switchToTask(TaskControlBlock* nextTask);
extern "C" void jumpRing3(void (*func)());

}  // namespace nyan::task
