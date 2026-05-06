#include "tty.hpp"

#include <nyan/errno.h>
#include <signal.h>

#include "../keyboard/forward.hpp"
#include "../task/pid.hpp"
#include "../task/scheduler.hpp"

namespace nyan::console {

void Tty::activate() noexcept {
    __flags |= F_Active;
    flush();
}

void Tty::deactivate() noexcept {
    __flags &= ~F_Active;
}

void Tty::input(const keyboard::Message& msg) noexcept {
    if (msg.flag & keyboard::F_Release) {
        return;
    }

    if (((msg.flag & keyboard::F_Modifiers) == keyboard::F_Ctrl) && msg.code == keyboard::SC_C) {
        if (__flags & F_Echo) {
            puts("^C\n", 3);
        }
        __line_buffer.clear();
        if (auto task = task::findTask(__foreground_pid)) {
            task::__scheduler->sendSignal(task, SIGINT);
        }
        return;
    }

    arch::InterruptGuard guard;
    if (__flags & F_Canonical) {
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
                        if (__line_buffer.length() > 0) {
                            __line_buffer.pop_back();
                            if (__flags & F_Echo) {
                                // 做的有点丑, 将就用下
                                putcImpl('\b');
                                putcImpl(' ');
                                putc('\b');
                            }
                        }
                    } else if (msg.ch == '\n') {
                        __line_buffer.push_back('\n');
                        __input_buffer.append(__line_buffer);
                        __line_buffer.clear();
                        if (__flags & F_Echo) {
                            putc('\n');
                        }
                    } else {
                        if (msg.flag & keyboard::F_Ctrl) {
                            if (msg.code == keyboard::SC_D) {
                                __input_buffer.append(__line_buffer);
                                __line_buffer.clear();
                                __pending_eof = true;
                            }
                        } else {
                            __line_buffer.push_back(msg.ch);
                            if (__flags & F_Echo) {
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
                __input_buffer.append("\x1B[A", 3);
                break;
            case keyboard::SC_DOWN:
                __input_buffer.append("\x1B[B", 3);
                break;
            case keyboard::SC_LEFT:
                __input_buffer.append("\x1B[D", 3);
                break;
            case keyboard::SC_RIGHT:
                __input_buffer.append("\x1B[C", 3);
                break;
            case keyboard::SC_DELETE:
                __input_buffer.append("\x1B[3~", 4);
                break;
            case keyboard::SC_HOME:
                __input_buffer.append("\x1B[H", 3);
                break;
            case keyboard::SC_END:
                __input_buffer.append("\x1B[F", 3);
                break;
            default:
                if (msg.ch) {
                    if (msg.flag & keyboard::F_Ctrl) {
                        __input_buffer.push_back(msg.ch & 0x1F);
                    } else {
                        __input_buffer.push_back(msg.ch);
                        if (__flags & F_Echo) {
                            putc(msg.ch);
                        }
                    }
                }
        }
    }

    __wait_list.wakeOne(task::WakeReason::WR_Normal);
}

bool Tty::inputEmpty() noexcept {
    arch::InterruptGuard guard;
    return __input_buffer.empty();
}

Result<arch::InterruptGuard> Tty::syncWaitInput() noexcept {
    while (true) {
        arch::InterruptGuard guard;
        if (!__input_buffer.empty()) {
            return guard;
        }
        if (__pending_eof) {
            __pending_eof = false;
            return guard;
        }
        if (__wait_list.wait(task::BlockReason::BR_WaitInput) == task::WakeReason::WR_Signal) {
            return SYS_EINTR;
        }
    }
}

}  // namespace nyan::console
