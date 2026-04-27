#pragma once

#include <optional>
#include <string>

#include "../arch/guard.hpp"
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

struct Tty : public ScreenBuffer {
    pid_t foregroundPid{task::KP_Invalid};

    std::string lineBuffer;

    std::string inputBuffer;
    task::WaitList waitList;

    bool pendingEof{};

    void activate();
    void deactivate();

    void input(const keyboard::Message& msg, interrupt::SyscallFrame* frame);

    bool inputEmpty();
    std::optional<arch::InterruptGuard> syncWaitInput();
};

}  // namespace nyan::console
