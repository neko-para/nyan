#include "signal.hpp"

namespace nyan::task {

void TaskSignalInfo::ensureActions() noexcept {
    if (!__signal_actions) {
        __signal_actions.reset(new SigData);
    }
}

void TaskSignalInfo::prepareForExec() noexcept {
    if (__signal_actions) {
        for (auto& act : *__signal_actions) {
            if (act.__handler != SIG_IGN && act.__handler != SIG_DFL) {
                act.__handler = SIG_DFL;
                act.__mask = 0;
                act.__flags = 0;
            }
        }
    }
}

void TaskSignalInfo::prepareForFork(TaskSignalInfo& current) noexcept {
    __pending_signals = 0;
    __signal_mask = current.__signal_mask;
    if (current.__signal_actions) {
        __signal_actions.reset(new SigData{*current.__signal_actions});
    } else {
        __signal_actions.reset();
    }
}

bool TaskSignalInfo::isInterrupted(int sig) const noexcept {
    if (!__signal_actions) {
        return !isSignalDefaultIgnore(sig);
    } else {
        const auto& entry = __signal_actions->operator[](sig);
        if (entry.__handler == SIG_IGN) {
            return false;
        } else if (entry.__handler == SIG_DFL) {
            return !isSignalDefaultIgnore(sig);
        } else {
            return true;
        }
    }
}

}  // namespace nyan::task