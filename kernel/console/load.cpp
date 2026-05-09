#include <fcntl.h>
#include <nyan/syscall.h>
#include <sys/wait.h>

#include "../fs/mod.hpp"
#include "../task/mod.hpp"
#include "../task/scheduler.hpp"
#include "../task/task.hpp"
#include "../task/tcb.hpp"
#include "device.hpp"
#include "mod.hpp"
#include "tty.hpp"

namespace nyan::console {

Tty* __active_tty;
Tty* __all_ttys[__tty_count];
TtyDevice* __all_tty_devices[__tty_count];

static int consoleDeamon(void* param) {
    Tty* tty = static_cast<Tty*>(param);
    auto id = std::find(std::begin(__all_ttys), std::end(__all_ttys), tty) - std::begin(__all_ttys);
    arch::kprint("tty {} deamon entered, pid {}\n", id, task::__scheduler->__current->pid);

    auto path = lib::format("/dev/tty{}", id);
    auto readFile = fs::open(path, O_RDONLY) | __unwrap;
    auto writeFile = fs::open(path, O_WRONLY) | __unwrap;

    task::installFileTo(readFile, 0) | __ignore;
    task::installFileTo(writeFile, 1) | __ignore;
    task::installFileTo(writeFile, 2) | __ignore;

    while (true) {
        const char* argv[] = {"sh", 0};
        const char* envp[] = {"PATH=/bin", "SHELL=/bin/sh", 0};
        auto pid = syscall::spawn("sh", argv, envp);
        tty->__foreground_pgid = pid;

        arch::kprint("tty {} shell started, pid {}\n", id, pid);

        auto [_, stat] = task::waitpid(pid, 0) | __unwrap;
        tty->print("\ntty {} shell exited, stat {} exit code {} signal {}\n", id, stat, WEXITSTATUS(stat),
                   WTERMSIG(stat));
    }
}

void load() noexcept {
    for (size_t i = 0; i < __tty_count; i++) {
        auto& tty = __all_ttys[i];
        auto& ttyDev = __all_tty_devices[i];

        tty = new Tty;
        tty->clear();
        ttyDev = new TtyDevice(tty);
    }
    __active_tty = __all_ttys[0];
    __active_tty->activate();
}

void startDeamons() noexcept {
    int id = 0;
    for (auto tty : __all_ttys) {
        auto tcb = task::createTask(consoleDeamon, tty);
        tcb->name = lib::format("tty_deamon_{}", id);
        task::__scheduler->addTask(tcb);
    }
}

void switchTo(Tty* tty) noexcept {
    if (tty->__flags & F_Active) {
        return;
    }

    arch::InterruptGuard guard;
    __active_tty->deactivate();
    __active_tty = tty;
    tty->activate();
}

void handleInput(const keyboard::Message& msg) noexcept {
    __active_tty->input(msg);
}

}  // namespace nyan::console
