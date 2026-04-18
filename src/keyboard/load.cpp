#include "load.hpp"

#include <sys/types.h>

#include "../interrupt/load.hpp"
#include "../tty/entry.hpp"
#include "message.hpp"

namespace nyan::keyboard {

static void waitReady() {
    while (arch::inb(0x64) & 0x2) {
        ;
    }
}

void load() {
    waitReady();
    arch::outb(0x64, 0x60);
    waitReady();
    arch::outb(0x60, 0x47);

    interrupt::unmask(1);
}

bool push(uint8_t dat) {
    static int state = 0;

    Message msg;

    switch (state) {
        case 0:
            if (dat == 0xE0) {
                state = 1;
                return false;
            } else {
                msg = merge(dat);
                tty::activeTty->input(msg);
                return true;
            }
        case 1:
            msg = merge(0xE000 | dat);
            state = 0;
            tty::activeTty->input(msg);
            return true;
        default:
            return false;
    }
}

}  // namespace nyan::keyboard
