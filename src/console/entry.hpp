#pragma once

#include <optional>

#include "../task/wait.hpp"
#include "buffer.hpp"

namespace nyan::keyboard {
struct Message;
}

namespace nyan::arch {
struct InterruptGuard;
}

namespace nyan::interrupt {
struct SyscallFrame;
}

namespace nyan::console {

constexpr size_t count = 2;

struct Tty : public ScreenBuffer {
    pid_t foregroundPid{task::KP_Invalid};

    lib::string lineBuffer;

    lib::string inputBuffer;
    task::WaitList waitList;

    void activate();
    void deactivate();

    void input(const keyboard::Message& msg, interrupt::SyscallFrame* frame);

    bool inputEmpty();
    std::optional<arch::InterruptGuard> syncWaitInput();
};

extern Tty* activeTty;
extern Tty* allTtys[count];

void load();
void loadDeamons();
void switchTo(Tty* tty);

}  // namespace nyan::console
