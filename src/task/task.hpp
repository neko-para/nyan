#pragma once

#include <stdint.h>

#include "../lib/containers.hpp"
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

struct ExitInfo {
    int code;
};

struct BlockSleepInfo {
    uint64_t time;
};

struct TaskControlBlock : public TaskControlBlockMetaInfo, public lib::ListBase<TaskControlBlockTag> {
    lib::vector<uint32_t> pages;
    // std::vector<uint32_t> pages;
    union {
        ExitInfo exitInfo;
        BlockSleepInfo sleepInfo;
    };

    void dump();
};

extern lib::List<TaskControlBlock> currentTask asm("currentTask");

void load();

extern "C" void switchToTask(TaskControlBlock* nextTask);

TaskControlBlock* createTask(int (*func)(void* param), void* param = nullptr);
pid_t addTask(TaskControlBlock* task);
__attribute__((noinline)) void initYield();
[[noreturn]] void exitTask(int code);

pid_t runTask(int (*func)(void* param), void* param = nullptr);
bool freeTask(pid_t pid, int* code);

__attribute__((noinline)) void yield();

void block(BlockReason reason);
void unblock(TaskControlBlock* task);

void sleep(uint64_t ms);
void checkSleep();

}  // namespace nyan::task
