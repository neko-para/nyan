#pragma once

#include <sys/types.h>

namespace nyan::task {

struct TaskControlBlock;

struct TaskControlBlockTag {
    using type = TaskControlBlock;
};

struct TaskControlBlockChildTag {
    using type = TaskControlBlock;
    constexpr static bool bidi = true;
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

enum KnownPid : pid_t {
    KP_Invalid = -1,

    KP_Init = 1,
    KP_Idle = 2,

    KP_FirstUser = 16,
};

}  // namespace nyan::task
