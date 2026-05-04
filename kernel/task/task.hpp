#pragma once

#include <stdint.h>
#include <sys/types.h>

#include "../interrupt/forward.hpp"
#include "forward.hpp"

namespace nyan::task {

TaskControlBlock* createTask(int (*func)(void* param), void* param = nullptr);
TaskControlBlock* createElfTask(uint8_t* file, size_t size, const char* const* argv, const char* const* envp);
void execTask(uint8_t* file,
              size_t size,
              const char* const* argv,
              const char* const* envp,
              interrupt::SyscallFrame* frame);
pid_t forkTask(interrupt::SyscallFrame* frame);
pid_t addTask(TaskControlBlock* task);
[[noreturn]] void exitTask(int code, int sig = 0);

bool freeTask(pid_t pid, int* code);

__attribute__((noinline)) void yield();

WakeReason block(BlockReason reason);
void unblock(TaskControlBlock* task, WakeReason reason);

WakeReason sleep(uint64_t ms, uint64_t* rest);
void checkSleep();

}  // namespace nyan::task
