#pragma once

#include <sys/types.h>

#include "../lib/containers.hpp"
#include "../lib/list.hpp"
#include "pid.hpp"

namespace nyan::task {

struct TaskControlBlock;
enum class BlockReason : uint16_t;

void load();

TaskControlBlock* createTask(int (*func)(void* param), void* param = nullptr);
TaskControlBlock* createElfTask(uint8_t* file, size_t size, const char* const* argv);
pid_t addTask(TaskControlBlock* task);
[[noreturn]] void exitTask(int code);

bool freeTask(pid_t pid, int* code);

__attribute__((noinline)) void yield();

void block(BlockReason reason);
void unblock(TaskControlBlock* task);

void sleep(uint64_t ms);
void checkSleep();

}  // namespace nyan::task
