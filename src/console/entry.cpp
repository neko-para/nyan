#include "entry.hpp"

#include <nyan/syscall.h>
#include <sys/wait.h>

#include "../arch/utils.hpp"
#include "../data/embed.hpp"
#include "../task/guard.hpp"

namespace nyan::console {

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
                        inputBuffer.pushSome(lineBuffer.data(), lineBuffer.size());
                        lineBuffer.clear();
                        if (flags & F_Echo) {
                            putc('\n');
                        }
                    } else {
                        if (msg.flag & keyboard::F_Ctrl) {
                            ;
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

int consoleDeamon(void* param) {
    Tty* tty = static_cast<Tty*>(param);

    while (true) {
        const char* argv[] = {"sh", 0};
        auto tcb = task::createElfTask(data::programs[0].data, data::programs[0].size, argv);
        tcb->tty = tty;
        auto pid = task::addTask(tcb);

        int stat = 0;
        syscall::waitpid(pid, &stat, 0);
        tty->print("\nshell exited, stat {} exit code {}\n", stat, WEXITSTATUS(stat));
    }
}

void load() {
    for (auto& tty : allTtys) {
        tty.currentAttr = vga::makeAttr(vga::C_LightGray, vga::C_Black);
        tty.flags = F_ShowCursor | F_Canonical | F_Echo;
        tty.clear();
    }
    activeTty = &allTtys[0];
    activeTty->activate();
}

void loadDeamons() {
    for (auto& tty : allTtys) {
        auto tcb = task::createTask(consoleDeamon, &tty);
        tcb->tty = &tty;
        task::addTask(tcb);
    }
}

void switchTo(Tty* tty) {
    if (tty->flags & F_Active) {
        return;
    }

    task::InterruptGuard guard;
    activeTty->deactivate();
    activeTty = tty;
    tty->activate();
}

}  // namespace nyan::console
