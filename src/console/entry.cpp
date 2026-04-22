#include "entry.hpp"

#include <nyan/syscall.h>
#include <sys/wait.h>

#include "../arch/guard.hpp"
#include "../arch/io.hpp"
#include "../keyboard/message.hpp"
#include "../task/task.hpp"
#include "../task/tcb.hpp"

namespace nyan::console {

Tty* activeTty;
Tty* allTtys[count];

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

    if (msg.flag & keyboard::F_Ctrl && msg.code == keyboard::SC_C) {
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
        if (waitList.wait(task::BlockReason::BR_WaitInput) == task::WakeReason::WR_Signal) {
            return std::nullopt;
        }
    }
}

int consoleDeamon(void* param) {
    Tty* tty = static_cast<Tty*>(param);
    auto id = std::find(std::begin(allTtys), std::end(allTtys), tty) - std::begin(allTtys);
    arch::kprint("tty {}: deamon entered, pid {}\n", id, task::currentTask->pid);

    task::currentTask->fdTable[0] = lib::makeRef<task::TtyObj>(tty, O_RDONLY);
    task::currentTask->fdTable[1] = lib::makeRef<task::TtyObj>(tty, O_WRONLY);
    task::currentTask->fdTable[2] = lib::makeRef<task::DebugConObj>();

    while (true) {
        const char* argv[] = {"sh", 0};
        auto pid = syscall::spawn("sh", argv);
        tty->foregroundPid = pid;

        arch::kprint("tty {}: shell started, pid {}\n", id, pid);

        int stat = 0;
        syscall::waitpid(pid, &stat, 0);
        tty->print("\ntty {}: shell exited, stat {} exit code {}\n", id, stat, WEXITSTATUS(stat));
    }
}

void load() {
    for (auto& tty : allTtys) {
        tty = allocator::frameAllocAs<Tty>();
        tty->clear();
    }
    activeTty = allTtys[0];
    activeTty->activate();
}

void loadDeamons() {
    int id = 0;
    for (auto tty : allTtys) {
        auto tcb = task::createTask(consoleDeamon, tty);
        tcb->name = lib::format("tty_deamon_{}", ++id);
        tcb->tty = tty;
        auto pid = task::addTask(tcb);
        arch::kprint("tty {} deamon started, pid {}\n", id, pid);
    }
}

void switchTo(Tty* tty) {
    if (tty->flags & F_Active) {
        return;
    }

    arch::InterruptGuard guard;
    activeTty->deactivate();
    activeTty = tty;
    tty->activate();
}

}  // namespace nyan::console
