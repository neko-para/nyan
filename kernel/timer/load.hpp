#pragma once

#include <stdint.h>
#include <sys/types.h>

namespace nyan::timer {

extern uint64_t msSinceBoot;

void load(uint32_t hz = 1000);
void hit();

}  // namespace nyan::timer
