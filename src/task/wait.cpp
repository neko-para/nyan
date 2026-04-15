#include "wait.hpp"

#include "guard.hpp"

namespace nyan::task {

void WaitList::wait() noexcept {
    InterruptGuard guard;
    list.pushBack(currentTask.head);
    block(BlockReason::BR_Wait);
}

bool WaitList::wakeOne() noexcept {
    InterruptGuard guard;
    if (auto task = list.popFront()) {
        unblock(task);
        return true;
    }
    return false;
}

void WaitList::wakeAll() noexcept {
    InterruptGuard guard;
    while (auto task = list.popFront()) {
        unblock(task);
    }
}

}  // namespace nyan::task
