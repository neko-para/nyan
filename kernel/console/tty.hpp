#pragma once

#include <string>

#include "../arch/guard.hpp"
#include "../keyboard/forward.hpp"
#include "../lib/result.hpp"
#include "../task/wait.hpp"
#include "buffer.hpp"

namespace nyan::console {

struct Tty : public ScreenBuffer {
    pid_t __foreground_pid{task::KP_Invalid};

    std::string __line_buffer;

    std::string __input_buffer;
    task::WaitList __wait_list;

    bool __pending_eof{};

    void activate() noexcept;
    void deactivate() noexcept;

    void input(const keyboard::Message& msg) noexcept;

    bool inputEmpty() noexcept;
    Result<arch::InterruptGuard> syncWaitInput() noexcept;
};

}  // namespace nyan::console
