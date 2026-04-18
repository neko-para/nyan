#include "load.hpp"

#include <sys/types.h>

#include "../arch/io.hpp"
#include "../interrupt/load.hpp"
#include "../lib/queue.hpp"
#include "../task/wait.hpp"
#include "../tty/entry.hpp"
#include "buffer.hpp"
#include "message.hpp"

namespace nyan::keyboard {

static void waitReady() {
    while (arch::inb(0x64) & 0x2) {
        ;
    }
}

lib::RingQueue<uint8_t, 256> buffer;
task::WaitList waitList;
bool echo = true;

void pushMsg(const Message& msg) {
    if (msg.flag & F_Release) {
        return;
    }

    tty::activeTty->input(msg);

    if (msg.flag & keyboard::F_Ctrl && msg.code == keyboard::SC_C) {
        arch::qemuQuit();
    }

    switch (msg.code) {
        case SC_UP:
            buffer.pushSome("\x1B[A", 3);
            break;
        case SC_DOWN:
            buffer.pushSome("\x1B[B", 3);
            break;
        case SC_LEFT:
            buffer.pushSome("\x1B[C", 3);
            break;
        case SC_RIGHT:
            buffer.pushSome("\x1B[D", 3);
            break;
        case SC_DELETE:
            buffer.pushSome("\x1B[3~", 4);
            break;
        case SC_HOME:
            buffer.pushSome("\x1B[H", 3);
            break;
        case SC_END:
            buffer.pushSome("\x1B[F", 3);
            break;
        default:
            if (msg.ch) {
                if (msg.flag & F_Ctrl) {
                    buffer.push(msg.ch & 0x1F);
                } else {
                    buffer.push(msg.ch);
                    if (echo) {
                        // TODO: echo
                        // vga::putc(msg.ch);
                    }
                }
            }
    }

    waitList.wakeOne();
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
                pushMsg(msg);
                return true;
            }
        case 1:
            msg = merge(0xE000 | dat);
            state = 0;
            pushMsg(msg);
            return true;
        default:
            return false;
    }
}

}  // namespace nyan::keyboard
