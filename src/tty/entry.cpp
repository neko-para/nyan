#include "entry.hpp"

#include "../arch/utils.hpp"
#include "../task/guard.hpp"

namespace nyan::tty {

Tty* activeTty;
Tty allTtys[count];

void Tty::activate() {
    flags |= F_Active;
    flush();
}

void Tty::deactivate() {
    flags &= ~F_Active;
}

void Tty::input(keyboard::Message msg) {
    if (msg.flag & keyboard::F_Release) {
        return;
    }

    if (msg.flag & keyboard::F_Ctrl && msg.code == keyboard::SC_C) {
        // TODO: kill current process
        arch::qemuQuit();
    }

    if (flags & F_Canonical) {
        switch (msg.code) {
            case keyboard::SC_UP:
                lineBuffer.append("\x1B[A");
                break;
            case keyboard::SC_DOWN:
                lineBuffer.append("\x1B[B");
                break;
            case keyboard::SC_LEFT:
                lineBuffer.append("\x1B[C");
                break;
            case keyboard::SC_RIGHT:
                lineBuffer.append("\x1B[D");
                break;
            case keyboard::SC_DELETE:
                lineBuffer.append("\x1B[3~");
                break;
            case keyboard::SC_HOME:
                lineBuffer.append("\x1B[H");
                break;
            case keyboard::SC_END:
                lineBuffer.append("\x1B[F");
                break;
            default:
                if (msg.ch) {
                    if (msg.ch == '\b') {
                        if (lineBuffer.length() > 0) {
                            lineBuffer.pop_back();
                            if (flags & F_Echo) {
                                // 做的有点丑, 将就用下
                                putcImpl('\b');
                                putcImpl(' ');
                                putc('\b');
                            }
                        }
                    } else if (msg.ch == '\n') {
                        lineBuffer.push_back('\n');
                        inputBuffer.pushSome(lineBuffer.data(), lineBuffer.size());
                        lineBuffer.clear();
                        if (flags & F_Echo) {
                            putc('\n');
                        }
                    } else {
                        if (msg.flag & keyboard::F_Ctrl) {
                            lineBuffer.push_back(msg.ch & 0x1F);
                        } else {
                            lineBuffer.push_back(msg.ch);
                            if (flags & F_Echo) {
                                putc(msg.ch);
                            }
                        }
                    }
                }
        }
    } else {
        switch (msg.code) {
            case keyboard::SC_UP:
                inputBuffer.pushSome("\x1B[A", 3);
                break;
            case keyboard::SC_DOWN:
                inputBuffer.pushSome("\x1B[B", 3);
                break;
            case keyboard::SC_LEFT:
                inputBuffer.pushSome("\x1B[C", 3);
                break;
            case keyboard::SC_RIGHT:
                inputBuffer.pushSome("\x1B[D", 3);
                break;
            case keyboard::SC_DELETE:
                inputBuffer.pushSome("\x1B[3~", 4);
                break;
            case keyboard::SC_HOME:
                inputBuffer.pushSome("\x1B[H", 3);
                break;
            case keyboard::SC_END:
                inputBuffer.pushSome("\x1B[F", 3);
                break;
            default:
                if (msg.ch) {
                    if (msg.flag & keyboard::F_Ctrl) {
                        inputBuffer.push(msg.ch & 0x1F);
                    } else {
                        inputBuffer.push(msg.ch);
                        if (flags & F_Echo) {
                            putc(msg.ch);
                        }
                    }
                }
        }
    }

    waitList.wakeOne();
}

bool Tty::inputEmpty() {
    task::InterruptGuard guard;
    return inputBuffer.empty();
}

void Tty::syncWaitInput() {
    while (inputEmpty()) {
        waitList.wait();
    }
}

void load() {
    for (auto& tty : allTtys) {
        tty.currentAttr = vga::makeAttr(vga::C_LightGray, vga::C_Black);
        tty.flags = F_ShowCursor | F_Canonical | F_Echo;
    }
    activeTty = &allTtys[0];
    activeTty->clear();
    activeTty->activate();
}

}  // namespace nyan::tty
