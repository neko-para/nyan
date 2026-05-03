#pragma once

#include <stdint.h>
#include <sys/types.h>

#include "forward.hpp"

namespace nyan::task {

extern "C" void switchToTask(TaskControlBlock* nextTask);
extern "C" [[noreturn]] void jumpRing3(uint32_t entry, uint32_t esp);
extern "C" [[noreturn]] void syscallReturn(void* esp);

}  // namespace nyan::task
