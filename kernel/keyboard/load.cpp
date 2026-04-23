#include "load.hpp"

#include <sys/types.h>

#include "../arch/port.hpp"
#include "../console/entry.hpp"
#include "../interrupt/load.hpp"
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

static void handle(const Message& msg, interrupt::SyscallFrame* frame) {
    if (!(msg.flag & F_Release) && (msg.flag & F_Ctrl) && (msg.flag & F_Alt)) {
        if (msg.key >= SC_F1 && msg.key <= SC_F2) {
            console::switchTo(console::allTtys[msg.key - SC_F1]);
            return;
        }
    }

    console::activeTty->input(msg, frame);
}

bool push(uint8_t dat, interrupt::SyscallFrame* frame) {
    static int state = 0;

    Message msg;

    switch (state) {
        case 0:
            if (dat == 0xE0) {
                state = 1;
                return false;
            } else {
                msg = merge(dat);
                handle(msg, frame);
                return true;
            }
        case 1:
            msg = merge(0xE000 | dat);
            state = 0;
            handle(msg, frame);
            return true;
        default:
            return false;
    }
}

}  // namespace nyan::keyboard
