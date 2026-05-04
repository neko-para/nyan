#pragma once

#include <stdint.h>
#include <sys/types.h>
#include <span>
#include <string>
#include <vector>

#include "../interrupt/forward.hpp"
#include "forward.hpp"

namespace nyan::task {

TaskControlBlock* createTask(int (*func)(void* param), void* param = nullptr);
TaskControlBlock* createElfTask(std::span<uint8_t> file,
                                std::vector<std::string> argv,
                                std::vector<std::string> env) noexcept;
pid_t forkTask(interrupt::SyscallFrame* frame);

}  // namespace nyan::task
