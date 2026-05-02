#pragma once

#include <optional>
#include <string>

#include "../arch/guard.hpp"
#include "../interrupt/forward.hpp"
#include "../keyboard/forward.hpp"
#include "../task/wait.hpp"
#include "buffer.hpp"

namespace nyan::console {

struct Tty : public ScreenBuffer {
    pid_t __foreground_pid{task::KP_Invalid};

    std::string __line_buffer;

    std::string __input_buffer;
    task::WaitList __wait_list;

    bool __pending_eof{};

    void activate();
    void deactivate();

    void input(const keyboard::Message& msg, interrupt::SyscallFrame* frame);

    bool inputEmpty();
    std::optional<arch::InterruptGuard> syncWaitInput();
};

}  // namespace nyan::console
