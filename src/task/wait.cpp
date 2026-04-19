#include "wait.hpp"

#include "../arch/guard.hpp"
#include "task.hpp"
#include "tcb.hpp"

namespace nyan::task {

void WaitList::wait(BlockReason reason) noexcept {
    arch::InterruptGuard guard;
    list.pushBack(currentTask.head);
    block(reason);
}

bool WaitList::wakeOne() noexcept {
    arch::InterruptGuard guard;
    if (auto task = list.popFront()) {
        unblock(task);
        return true;
    }
    return false;
}

void WaitList::wakeAll() noexcept {
    arch::InterruptGuard guard;
    while (auto task = list.popFront()) {
        unblock(task);
    }
}

}  // namespace nyan::task
