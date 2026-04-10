#pragma once

#include <stdint.h>

#include "../lib/list.hpp"
#include "pid.hpp"

namespace nyan::task {

struct TaskControlBlock;

struct TaskControlBlockTag {
    using type = TaskControlBlock;
};

enum class State : uint16_t {
    S_Ready,
    S_Running,
    S_Exited,

    S_Blocked,
};

enum class BlockReason : uint16_t {
    BR_Unknown,
    BR_Sleep,
};

struct TaskControlBlockMetaInfo {
    uint32_t userEsp;
    uint32_t cr3;
    uint32_t kernelEsp;
    State state;
    BlockReason blockReason;
    pid_t pid;
};

struct BlockSleepInfo {
    uint64_t time;
};

struct TaskControlBlock : public TaskControlBlockMetaInfo, public lib::ListBase<TaskControlBlockTag> {
    union {
        BlockSleepInfo sleepInfo;
    };
};

extern lib::List<TaskControlBlock> currentTask asm("currentTask");

void load();

extern "C" void switchToTask(TaskControlBlock* nextTask);

TaskControlBlock* createTask(void (*func)(void* param), void* param);
pid_t addTask(TaskControlBlock* task);
void initYield();

void yield();

void block(BlockReason reason);
void unblock(TaskControlBlock* task);

void sleep(uint64_t ms);
void checkSleep();

}  // namespace nyan::task
