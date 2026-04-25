#include "tty.hpp"

#include <signal.h>

#include "../keyboard/message.hpp"
#include "../task/task.hpp"

namespace nyan::console {

void Tty::activate() {
    flags |= F_Active;
    flush();
}

void Tty::deactivate() {
    flags &= ~F_Active;
}

void Tty::input(const keyboard::Message& msg, interrupt::SyscallFrame*) {
    if (msg.flag & keyboard::F_Release) {
        return;
    }

    if (((msg.flag & keyboard::F_Modifiers) == keyboard::F_Ctrl) && msg.code == keyboard::SC_C) {
        if (flags & F_Echo) {
            puts("^C\n", 3);
        }
        lineBuffer.clear();
        if (auto task = task::findTask(foregroundPid)) {
            task::sendSignal(task, SIGINT);
        }
        return;
    }

    arch::InterruptGuard guard;
    if (flags & F_Canonical) {
        switch (msg.code) {
            case keyboard::SC_UP:
            case keyboard::SC_DOWN:
            case keyboard::SC_LEFT:
            case keyboard::SC_RIGHT:
            case keyboard::SC_DELETE:
            case keyboard::SC_HOME:
            case keyboard::SC_END:
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
                        inputBuffer.append(lineBuffer);
                        lineBuffer.clear();
                        if (flags & F_Echo) {
                            putc('\n');
                        }
                    } else {
                        if (msg.flag & keyboard::F_Ctrl) {
                            if (msg.code == keyboard::SC_D) {
                                inputBuffer.append(lineBuffer);
                                lineBuffer.clear();
                                pendingEof = true;
                            }
                        } else {
                            lineBuffer.push_back(msg.ch);
                            if (flags & F_Echo) {
                                putc(msg.ch);
                            }
                            // TODO: 不要wake
                        }
                    }
                }
        }
    } else {
        switch (msg.code) {
            case keyboard::SC_UP:
                inputBuffer.append("\x1B[A", 3);
                break;
            case keyboard::SC_DOWN:
                inputBuffer.append("\x1B[B", 3);
                break;
            case keyboard::SC_LEFT:
                inputBuffer.append("\x1B[D", 3);
                break;
            case keyboard::SC_RIGHT:
                inputBuffer.append("\x1B[C", 3);
                break;
            case keyboard::SC_DELETE:
                inputBuffer.append("\x1B[3~", 4);
                break;
            case keyboard::SC_HOME:
                inputBuffer.append("\x1B[H", 3);
                break;
            case keyboard::SC_END:
                inputBuffer.append("\x1B[F", 3);
                break;
            default:
                if (msg.ch) {
                    if (msg.flag & keyboard::F_Ctrl) {
                        inputBuffer.push_back(msg.ch & 0x1F);
                    } else {
                        inputBuffer.push_back(msg.ch);
                        if (flags & F_Echo) {
                            putc(msg.ch);
                        }
                    }
                }
        }
    }

    waitList.wakeOne(task::WakeReason::WR_Normal);
}

bool Tty::inputEmpty() {
    arch::InterruptGuard guard;
    return inputBuffer.empty();
}

std::optional<arch::InterruptGuard> Tty::syncWaitInput() {
    while (true) {
        arch::InterruptGuard guard;
        if (!inputBuffer.empty()) {
            return guard;
        }
        if (pendingEof) {
            pendingEof = false;
            return guard;
        }
        if (waitList.wait(task::BlockReason::BR_WaitInput) == task::WakeReason::WR_Signal) {
            return std::nullopt;
        }
    }
}

}  // namespace nyan::console
