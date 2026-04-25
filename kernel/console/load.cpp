#include "load.hpp"

#include <fcntl.h>
#include <nyan/syscall.h>
#include <sys/wait.h>

#include "../arch/file.hpp"
#include "../task/task.hpp"
#include "../task/tcb.hpp"
#include "file.hpp"
#include "tty.hpp"

namespace nyan::console {

Tty* activeTty;
Tty* allTtys[count];

static int consoleDeamon(void* param) {
    Tty* tty = static_cast<Tty*>(param);
    auto id = std::find(std::begin(allTtys), std::end(allTtys), tty) - std::begin(allTtys);
    arch::kprint("tty {}: deamon entered, pid {}\n", id, task::currentTask->pid);

    auto ttyObj = lib::makeRef<TtyObj>(tty);
    auto debugConObj = lib::makeRef<arch::DebugConObj>();
    task::currentTask->fdTable[0] = lib::makeRef<fs::FdObj>(ttyObj, O_RDONLY);
    task::currentTask->fdTable[1] = lib::makeRef<fs::FdObj>(ttyObj, O_WRONLY);
    task::currentTask->fdTable[2] = lib::makeRef<fs::FdObj>(debugConObj, O_WRONLY);

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
