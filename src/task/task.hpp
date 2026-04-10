#pragma once

#include <stdint.h>

#include "../lib/list.hpp"

namespace nyan::task {

struct TaskControlBlock;

struct TaskControlBlockTag {
    using type = TaskControlBlock;
};

struct TaskControlBlockMetaInfo {
    uint32_t userEsp;
    uint32_t cr3;
    uint32_t kernelEsp;
};

struct TaskControlBlock : public TaskControlBlockMetaInfo, public lib::ListBase<TaskControlBlockTag> {};

extern lib::List<TaskControlBlock> currentTask;

extern "C" void switchToTask(TaskControlBlock* nextTask);

TaskControlBlock* createTask(void (*func)(void* param), void* param);
void addTask(TaskControlBlock* task);
void initYield();

}  // namespace nyan::task
