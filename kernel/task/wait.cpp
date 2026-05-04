#include "wait.hpp"

#include "../arch/guard.hpp"
#include "scheduler.hpp"
#include "task.hpp"
#include "tcb.hpp"

namespace nyan::task {

WakeReason WaitList::wait(BlockReason reason) noexcept {
    arch::InterruptGuard guard;
    __list.push_back(__scheduler->__current);
    __scheduler->__current->__request_detach = [this](TaskControlBlock* task) { __list.erase({task}); };
    return block(reason);
}

bool WaitList::wakeOne(WakeReason reason) noexcept {
    arch::InterruptGuard guard;
    if (!__list.empty()) {
        auto task = __list.front();
        __list.pop_front();
        task->__request_detach.reset();
        unblock(task, reason);
        return true;
    }
    return false;
}

void WaitList::wakeAll(WakeReason reason) noexcept {
    arch::InterruptGuard guard;
    while (!__list.empty()) {
        auto task = __list.front();
        __list.pop_front();
        task->__request_detach.reset();
        unblock(task, reason);
    }
}

}  // namespace nyan::task
