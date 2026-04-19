#pragma once

#include <sys/signal.h>
#include <sys/types.h>

#include "../lib/containers.hpp"
#include "../lib/list.hpp"
#include "forward.hpp"
#include "wait.hpp"

namespace nyan::console {
struct Tty;
}

namespace nyan::task {

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

struct TaskControlBlock : public TaskControlBlockMetaInfo,
                          public lib::ListBase<TaskControlBlockTag, TaskControlBlockChildTag> {
    pid_t parentPid{KP_Invalid};
    pid_t groupPid{KP_Invalid};
    lib::TailList<TaskControlBlockChildTag> childTasks;

    sigset_t pendingSignals{};
    sigset_t signalMask{};
    lib::unique_ptr<std::array<sigaction, NSIG>> signalActions;

    lib::string name;
    paging::VirtualAddress brkAddr;
    console::Tty* tty{};
    lib::vector<uint32_t> pages;
    WaitList wait;
    union {
        ExitInfo exitInfo;
        BlockSleepInfo sleepInfo;
        BlockWaitInfo waitInfo;
    };

    bool ended() const noexcept { return state == State::S_Exited; }
    void dump();
};

extern lib::List<TaskControlBlockTag> currentTask asm("currentTask");

}  // namespace nyan::task
