#pragma once

#include <sys/types.h>

#include "forward.hpp"

namespace nyan::console {

constexpr size_t count = 2;

extern Tty* activeTty;
extern Tty* allTtys[count];

void load();
void loadDeamons();
void switchTo(Tty* tty);

}  // namespace nyan::console
