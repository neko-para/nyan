#pragma once

#include <nyan/signal.h>
#include <signal.h>
#include <array>
#include <memory>

namespace nyan::task {

struct TaskSignalInfo {
    using SigData = std::array<__nyan_sigaction, NSIG>;

    __nyan_sigset __pending_signals{};
    __nyan_sigset __signal_mask{};
    std::unique_ptr<SigData> __signal_actions;

    bool isMasked(int sig) const noexcept { return __signal_mask & (1ull << (sig - 1)); }
    __nyan_sigset restSignals() const noexcept { return __pending_signals & (~__signal_mask); }

    void ensureActions() noexcept;
    void prepareForExec() noexcept;
    void prepareForFork(TaskSignalInfo& current) noexcept;

    bool isInterrupted(int sig) const noexcept;
};

inline bool isSignalDefaultIgnore(int sig) {
    return sig == SIGCHLD || sig == SIGURG || sig == SIGWINCH;
}

}  // namespace nyan::task
