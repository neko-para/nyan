#pragma once

#include <stdint.h>
#include <sys/types.h>

#include "../lib/lazy.hpp"

namespace nyan::task {

struct TaskControlBlock;
struct Scheduler;

struct TaskControlBlockTag {
    using type = TaskControlBlock;
};

struct TaskControlBlockChildTag {
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

enum class WakeReason : uint16_t {
    WR_Normal,
    WR_Signal,
};

enum KnownPid : pid_t {
    KP_Invalid = -1,

    KP_Init = 1,
    KP_Idle = 2,

    KP_FirstUser = 16,
};

// TODO: 这玩意是不是要动态?
constexpr size_t __max_fd = 16;

constexpr pid_t __max_task = 256;

extern lib::Lazy<Scheduler> __scheduler asm("scheduler");

}  // namespace nyan::task
