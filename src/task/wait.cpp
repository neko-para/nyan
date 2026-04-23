#include "wait.hpp"

#include "../arch/guard.hpp"
#include "task.hpp"
#include "tcb.hpp"

namespace nyan::task {

WakeReason WaitList::wait(BlockReason reason) noexcept {
    arch::InterruptGuard guard;
    list.push_back(currentTask);
    currentTask->requestDetach = [this](TaskControlBlock* task) { list.erase({task}); };
    block(reason);
    auto wakeReason = currentTask->wakeReason;
    currentTask->wakeReason = WakeReason::WR_Normal;
    return wakeReason;
}

bool WaitList::wakeOne(WakeReason reason) noexcept {
    arch::InterruptGuard guard;
    if (!list.empty()) {
        auto task = list.front();
        list.pop_front();
        task->requestDetach.reset();
        unblock(task, reason);
        return true;
    }
    return false;
}

void WaitList::wakeAll(WakeReason reason) noexcept {
    arch::InterruptGuard guard;
    while (!list.empty()) {
        auto task = list.front();
        list.pop_front();
        task->requestDetach.reset();
        unblock(task, reason);
    }
}

}  // namespace nyan::task
