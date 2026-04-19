#pragma once

#include "../task/wait.hpp"
#include "buffer.hpp"

namespace nyan::keyboard {
struct Message;
}

namespace nyan::arch {
struct InterruptGuard;
}

namespace nyan::console {

constexpr size_t count = 2;

struct Tty : public ScreenBuffer {
    // pid_t currentPid;

    lib::string lineBuffer;

    lib::string inputBuffer;
    task::WaitList waitList;

    void activate();
    void deactivate();

    void input(const keyboard::Message& msg);

    bool inputEmpty();
    arch::InterruptGuard syncWaitInput();
};

extern Tty* activeTty;
extern Tty* allTtys[count];

void load();
void loadDeamons();
void switchTo(Tty* tty);

}  // namespace nyan::console
