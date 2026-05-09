#include "scheduler.hpp"

#include "../arch/guard.hpp"
#include "../timer/load.hpp"
#include "tcb.hpp"

namespace nyan::task {

void Scheduler::wake(TaskControlBlock* task, WakeReason reason) noexcept {
    if (task->state != State::S_Blocked) {
        return;
    }
    task->state = State::S_Ready;
    task->blockReason = BlockReason::BR_Unknown;
    task->wakeReason = reason;
    if (auto func = std::move(task->__request_detach)) {
        task->__request_detach.reset();
        func(task);
    }

    {
        arch::InterruptGuard guard;
        __pending.push_back(task);
    }
}

void Scheduler::checkSleep() noexcept {
    arch::InterruptGuard guard;
    while (!__sleeping.empty()) {
        auto task = __sleeping.front();
        if (task->sleepInfo.time < timer::msSinceBoot) {
            __sleeping.pop_front();
            task->__request_detach.reset();
            wake(task, WakeReason::WR_Normal);
        } else {
            break;
        }
    }
    if ((timer::msSinceBoot % 10 == 0) && __current) {
        yield();
    }
}

void Scheduler::sendSignal(TaskControlBlock* task, int sig) noexcept {
    arch::InterruptGuard guard;
    if (task->ended()) {
        arch::kprint("signal {} ignored for pid {} as it is ended\n", sig, task->pid);
        return;
    } else {
        arch::kprint("send signal {} to pid {}\n", sig, task->pid);
    }
    task->__signal.__pending_signals |= 1ull << sig;
    if (task->__signal.isMasked(sig)) {
        arch::kprint("signal {} masked for pid {}\n", sig, task->pid);
        return;
    }
    if (task->state == State::S_Blocked) {
        wake(task, WakeReason::WR_Signal);
    }
}

}  // namespace nyan::task
