#pragma once

#include <signal.h>
#include <stdint.h>
#include <sys/types.h>
#include <string>
#include <utility>
#include <vector>

#include "../fs/dentry.hpp"
#include "../gdt/entry.hpp"
#include "../lib/function.hpp"
#include "../lib/list.hpp"
#include "../paging/vma.hpp"
#include "forward.hpp"
#include "tcb/file.hpp"
#include "tcb/signal.hpp"
#include "wait.hpp"

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

struct TaskControlBlock : public TaskControlBlockMetaInfo,
                          public lib::ListNodes<TaskControlBlockTag, TaskControlBlockChildTag> {
    paging::VMSpace vmSpace;

    pid_t parentPid{KP_Invalid};
    pid_t groupPid{KP_Invalid};
    lib::List<TaskControlBlockChildTag, true> childTasks;

    TaskSignalInfo __signal;
    TaskFileInfo __file;

    gdt::Segment tls;

    std::string name;
    paging::VirtualAddress brkBase;
    paging::VirtualAddress brkAddr;
    std::pair<paging::VirtualAddress, paging::VirtualAddress> stackRange;
    std::vector<uint32_t> pages;

    lib::Ref<fs::DEntry> cwd;
    // std::vector<std::string> argv;
    // std::vector<std::string> env;

    union {
        ExitInfo exitInfo;
        BlockSleepInfo sleepInfo;
        BlockWaitTaskInfo waitTaskInfo;
    };
    lib::function<void(TaskControlBlock*)> requestDetach;

    bool ended() const noexcept { return state == State::S_Exited; }
    void dump();

    void sendSignal(int sig) noexcept;
    bool peekSignal() const noexcept { return __signal.peek(); }
    // 几乎总是应该用 currentTask 来调用
    bool checkSignal(interrupt::SyscallFrame* frame) noexcept;
};

extern TaskControlBlock* currentTask asm("currentTask");

}  // namespace nyan::task
