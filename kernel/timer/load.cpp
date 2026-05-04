#include "load.hpp"

#include "../arch/port.hpp"
#include "../interrupt/load.hpp"
#include "../task/scheduler.hpp"

namespace nyan::timer {

constexpr uint32_t defaultHz = 1'193'182;

uint64_t msSinceBoot;

void load(uint32_t hz) {
    uint16_t h = defaultHz / hz;
    arch::outb(0x43, 0x34);
    arch::outb(0x40, h & 0xFF);
    arch::outb(0x40, h >> 8);

    interrupt::unmask(0);
}

void hit() {
    msSinceBoot += 1;

    task::__scheduler->checkSleep();
}

}  // namespace nyan::timer
