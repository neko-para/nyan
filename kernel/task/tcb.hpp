#pragma once

#include <sys/signal.h>
#include <sys/types.h>

#include "../lib/containers.hpp"
#include "../lib/function.hpp"
#include "../lib/list.hpp"
#include "fd.hpp"
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
    WakeReason wakeReason;
    pid_t pid;
};

struct ExitInfo {
    int stat;
};

struct BlockSleepInfo {
    uint64_t time;
};

struct BlockWaitTaskInfo {
    pid_t pid;
};

// TODO: 这玩意是不是要动态?
constexpr size_t MAXFD = 16;

struct TaskControlBlock : public TaskControlBlockMetaInfo,
                          public lib::ListNodes<TaskControlBlockTag, TaskControlBlockChildTag> {
    pid_t parentPid{KP_Invalid};
    pid_t groupPid{KP_Invalid};
    lib::List<TaskControlBlockChildTag, true> childTasks;

    sigset_t pendingSignals{};
    sigset_t signalMask{};
    lib::unique_ptr<std::array<sigaction, NSIG>> signalActions;

    std::array<lib::Ref<FdObj>, MAXFD> fdTable;

    lib::string name;
    paging::VirtualAddress brkAddr;
    console::Tty* tty{};
    lib::vector<uint32_t> pages;

    union {
        ExitInfo exitInfo;
        BlockSleepInfo sleepInfo;
        BlockWaitTaskInfo waitTaskInfo;
    };
    lib::function<void(TaskControlBlock*)> requestDetach;

    bool ended() const noexcept { return state == State::S_Exited; }
    void dump();
};

extern TaskControlBlock* currentTask asm("currentTask");

}  // namespace nyan::task
