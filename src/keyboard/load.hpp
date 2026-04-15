#pragma once

#include <stdint.h>

namespace nyan::keyboard {

struct Message;

void load();
bool push(uint8_t dat);

}  // namespace nyan::keyboard
