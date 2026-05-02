#pragma once

#include <signal.h>
#include <stdint.h>
#include <sys/types.h>
#include <array>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "../fs/dentry.hpp"
#include "../fs/fd.hpp"
#include "../gdt/entry.hpp"
#include "../lib/function.hpp"
#include "../lib/list.hpp"
#include "../paging/vma.hpp"
#include "forward.hpp"
#include "signal.hpp"
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
    paging::VMSpace vmSpace;

    pid_t parentPid{KP_Invalid};
    pid_t groupPid{KP_Invalid};
    lib::List<TaskControlBlockChildTag, true> childTasks;

    SigSet pendingSignals{};
    SigSet signalMask{};
    std::unique_ptr<std::array<SigAction, NSIG>> signalActions;

    std::array<lib::Ref<fs::FdObj>, MAXFD> fdTable;
    console::Tty* tty{};

    gdt::Segment tls;

    std::string name;
    paging::VirtualAddress brkBase;
    paging::VirtualAddress brkAddr;
    std::pair<paging::VirtualAddress, paging::VirtualAddress> stackRange;
    std::vector<uint32_t> pages;

    lib::Ref<fs::DEntry> cwd;
    // std::vector<std::string> env;

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
