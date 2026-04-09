#pragma once

#include <stdint.h>

namespace nyan::task {

struct TaskControlBlock {
    uint32_t userEsp;
    uint32_t cr3;
    uint32_t kernelEsp;

    TaskControlBlock* next;
};

extern TaskControlBlock* currentTask;

extern "C" void switchToTask(TaskControlBlock* nextTask);

TaskControlBlock* createTask(void (*func)(void* param), void* param);
void initYield(TaskControlBlock* task);

}  // namespace nyan::task
