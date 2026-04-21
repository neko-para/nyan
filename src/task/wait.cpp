#include "wait.hpp"

#include "../arch/guard.hpp"
#include "task.hpp"
#include "tcb.hpp"

namespace nyan::task {

WakeReason WaitList::wait(BlockReason reason) noexcept {
    arch::InterruptGuard guard;
    list.pushBack(currentTask.head);
    currentTask->blockWaitTarget = this;
    block(reason);
    auto wakeReason = currentTask->wakeReason;
    currentTask->wakeReason = WakeReason::WR_Normal;
    return wakeReason;
}

bool WaitList::wakeOne(WakeReason reason) noexcept {
    arch::InterruptGuard guard;
    if (auto task = list.popFront()) {
        task->blockWaitTarget = {};
        unblock(task, reason);
        return true;
    }
    return false;
}

void WaitList::wakeAll(WakeReason reason) noexcept {
    arch::InterruptGuard guard;
    while (auto task = list.popFront()) {
        task->blockWaitTarget = {};
        unblock(task, reason);
    }
}

void WaitList::take(TaskControlBlock* tcb) noexcept {
    arch::InterruptGuard guard;
    // TODO: 这里为了能take改成了双向链表. 重写下链表的能力, 搞个搜索-删除
    list.take(tcb);
}

}  // namespace nyan::task
