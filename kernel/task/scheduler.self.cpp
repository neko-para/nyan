#include "scheduler.hpp"

#include "../arch/guard.hpp"
#include "../gdt/load.hpp"
#include "../timer/load.hpp"
#include "pid.hpp"
#include "switch.hpp"
#include "task.hpp"
#include "tcb.hpp"
#include "trampoline.hpp"

namespace nyan::task {

void Scheduler::exit(int code, int sig) noexcept {
    arch::cli();

    if (__current->pid == KP_Init) {
        arch::kfatal("init task cannot exit!");
    }

    __current->state = State::S_Exited;
    __current->exitInfo.stat = (code << 8) | sig;
    if (!__current->childTasks.empty()) {
        __all_tasks[KP_Init]->childTasks.splice(__all_tasks[KP_Init]->childTasks.end(), __current->childTasks);
        sendSignal(__all_tasks[KP_Init], SIGCHLD);
    }
    if (auto parent = findTask(__current->parentPid)) {
        sendSignal(parent, SIGCHLD);
    }
    if (!__pending.empty()) {
        auto task = __pending.front();
        __pending.pop_front();
        switchToTask(task);
    } else {
        switchToTask(__all_tasks[KP_Idle]);
    }
    arch::kfatal("exited task rescheduled!");
}

void Scheduler::yield() noexcept {
    arch::InterruptGuard guard;
    if (!__pending.empty()) {
        auto next = __pending.front();
        __pending.pop_front();
        if (__current->state == State::S_Running) {
            __current->state = State::S_Ready;
            if (__current->pid != KP_Idle) {
                __pending.push_back(__current);
            }
        }
        switchToTask(next);
        __current->state = State::S_Running;
        gdt::setTls(__current->tls);
    } else if (__current->state != State::S_Running) {
        switchToTask(__all_tasks[KP_Idle]);
        __current->state = State::S_Running;
        gdt::setTls(__current->tls);
    }
}

WakeReason Scheduler::block(BlockReason reason) noexcept {
    arch::InterruptGuard guard;
    __current->state = State::S_Blocked;
    __current->blockReason = reason;
    __current->wakeReason = WakeReason::WR_Normal;
    yield();
    auto wr = __current->wakeReason;
    __current->wakeReason = WakeReason::WR_Normal;
    return wr;
}

WakeReason Scheduler::sleep(uint64_t ms, uint64_t* rest) noexcept {
    auto currTs = timer::msSinceBoot + ms;

    arch::InterruptGuard guard;
    __current->sleepInfo.time = currTs;

    auto pos = std::find_if(__sleeping.begin(), __sleeping.end(),
                            [&](const auto& tcb) { return currTs <= tcb.sleepInfo.time; });
    __sleeping.insert(pos, __current);
    __current->__request_detach = +[](TaskControlBlock* task) { __scheduler->__sleeping.erase({task}); };
    auto reason = block(BlockReason::BR_Sleep);
    if (rest) {
        if (currTs <= timer::msSinceBoot) {
            *rest = 0;
        } else {
            *rest = currTs - timer::msSinceBoot;
        }
    }
    return reason;
}

static void defaultSignalLogic(int sig) {
    if (isSignalDefaultIgnore(sig)) {
        return;
    }
    __scheduler->exit(255, sig);
}

bool Scheduler::checkSignal(interrupt::SyscallFrame* frame) noexcept {
    // TODO: 假设一定来自用户态. 需要有个地方检查frame->cs
    arch::InterruptGuard guard;
    SigSet sigs = __current->__signal.restSignals();
    if (!sigs) {
        return false;
    }
    auto sig = std::countr_zero(sigs);
    __current->__signal.__pending_signals ^= 1ull << sig;
    if (!__current->__signal.__signal_actions) {
        defaultSignalLogic(sig);
    } else {
        const auto& entry = __current->__signal.__signal_actions->operator[](sig);
        if (entry.__handler == SIG_IGN) {
            ;
        } else if (entry.__handler == SIG_DFL) {
            defaultSignalLogic(sig);
        } else {
            auto mask = __current->__signal.__signal_mask;
            __current->__signal.__signal_mask |= entry.__mask;

            // TODO: 这里关闭了中断, 需要检查是否跨页了.
            auto esp = frame->user_esp;
            esp -= sizeof(task::SignalFrame);
            auto userFrame = reinterpret_cast<task::SignalFrame*>(esp);
            userFrame->retAddr = (0xFFFFF000_va + (sigreturn_trampoline - trampoline_start)).addr;
            userFrame->signal = sig;
            userFrame->oldMask = mask;
            userFrame->frame = *frame;

            frame->eip = reinterpret_cast<uint32_t>(entry.__handler);
            frame->user_esp = esp;
        }
    }
    return true;
}

bool Scheduler::isInterrupted() const noexcept {
    SigSet sigs = __current->__signal.restSignals();
    while (sigs) {
        auto sig = std::countr_zero(sigs);
        sigs ^= 1ull << sig;
        if (__current->__signal.isInterrupted(sig)) {
            return true;
        }
    }
    return false;
}

}  // namespace nyan::task
