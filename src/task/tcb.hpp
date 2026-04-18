#pragma once

#include <sys/types.h>

#include "../lib/containers.hpp"
#include "../lib/list.hpp"

namespace nyan::console {
struct Tty;
}

namespace nyan::task {

struct TaskControlBlock;

struct TaskControlBlockTag {
    using type = TaskControlBlock;
};

enum class State : uint16_t {
    S_Ready,
    S_Running,
    S_Exited,
    // TODO: signal

    S_Blocked,
};

enum class BlockReason : uint16_t {
    BR_Unknown,
    BR_Sleep,
    BR_WaitInput,
    BR_WaitTask,
};

struct TaskControlBlockMetaInfo {
    uint32_t userEsp;
    paging::PhysicalAddress cr3;
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

struct BlockWaitInfo {
    pid_t pid;
};

struct TaskControlBlock : public TaskControlBlockMetaInfo, public lib::ListBase<TaskControlBlockTag> {
    lib::string name;
    paging::VirtualAddress brkAddr;
    console::Tty* tty{};
    lib::vector<uint32_t> pages;
    lib::List<TaskControlBlock> waitingTasks;
    union {
        ExitInfo exitInfo;
        BlockSleepInfo sleepInfo;
        BlockWaitInfo waitInfo;
    };

    bool ended() const noexcept { return state == State::S_Exited; }
    void dump();
};

extern lib::List<TaskControlBlock> currentTask asm("currentTask");

}  // namespace nyan::task
