#pragma once

#include <stdint.h>
#include <sys/types.h>

namespace nyan::interrupt {

void load();

void mask(uint8_t irq);
void unmask(uint8_t irq);
void end(uint8_t irq);

}  // namespace nyan::interrupt
