#pragma once

#include <sys/types.h>

namespace nyan::interrupt {
struct SyscallFrame;
}

namespace nyan::keyboard {

struct Message;

void load();
bool push(uint8_t dat, interrupt::SyscallFrame* frame);

}  // namespace nyan::keyboard
