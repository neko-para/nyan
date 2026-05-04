#pragma once

#include <signal.h>
#include <array>
#include <memory>

#include "../forward.hpp"
#include "../signal.hpp"

namespace nyan::task {

struct TaskSignalInfo {
    using SigData = std::array<SigAction, NSIG>;

    SigSet __pending_signals{};
    SigSet __signal_mask{};
    std::unique_ptr<SigData> __signal_actions;

    bool isMasked(int sig) const noexcept { return __signal_mask & (1ull << sig); }
    SigSet restSignals() const noexcept { return __pending_signals & (~__signal_mask); }

    void ensureActions() noexcept;
    void prepareForExec() noexcept;
    void prepareForFork(TaskSignalInfo& current) noexcept;

    bool isInterrupted(int sig) const noexcept;
};

inline bool isSignalDefaultIgnore(int sig) {
    return sig == SIGCHLD || sig == SIGURG || sig == SIGWINCH;
}

}  // namespace nyan::task