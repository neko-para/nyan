#include "wait.hpp"

#include "../arch/guard.hpp"
#include "task.hpp"
#include "tcb.hpp"

namespace nyan::task {

WakeReason WaitList::wait(BlockReason reason) noexcept {
    arch::InterruptGuard guard;
    list.pushBack(currentTask.head);
    block(reason);
    auto wakeReason = currentTask->wakeReason;
    currentTask->wakeReason = WakeReason::WR_Normal;
    return wakeReason;
}

bool WaitList::wakeOne(WakeReason reason) noexcept {
    arch::InterruptGuard guard;
    if (auto task = list.popFront()) {
        unblock(task, reason);
        return true;
    }
    return false;
}

void WaitList::wakeAll(WakeReason reason) noexcept {
    arch::InterruptGuard guard;
    while (auto task = list.popFront()) {
        unblock(task, reason);
    }
}

}  // namespace nyan::task
