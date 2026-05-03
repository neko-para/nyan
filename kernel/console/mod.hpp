#pragma once

#include <sys/types.h>

#include "../keyboard/forward.hpp"
#include "forward.hpp"

namespace nyan::console {

constexpr size_t __tty_count = 2;

extern Tty* __active_tty;
extern Tty* __all_ttys[__tty_count];

void load() noexcept;
void startDeamons() noexcept;
void switchTo(Tty* tty) noexcept;
void handleInput(const keyboard::Message& msg) noexcept;

}  // namespace nyan::console