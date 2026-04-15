#pragma once

#include <stdint.h>

#include "../lib/queue.hpp"
#include "../task/wait.hpp"

namespace nyan::keyboard {

extern lib::RingQueue<uint8_t, 256> buffer;
extern task::WaitList waitList;

}  // namespace nyan::keyboard
