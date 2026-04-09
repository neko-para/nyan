#pragma once

#include <stdint.h>

namespace nyan::keyboard {

struct Message;

using KeyboardCallback = void (*)(const Message& msg);

extern KeyboardCallback keyboardCallback;

void load();
bool push(uint8_t dat);

}  // namespace nyan::keyboard
