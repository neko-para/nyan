#include "tcb.hpp"

#include <sys/wait.h>

#include "../timer/load.hpp"
#include "scheduler.hpp"
#include "task.hpp"
#include "trampoline.hpp"

namespace nyan::task {

void TaskControlBlock::dump() {
    switch (state) {
        case State::S_Ready:
            arch::kprint("task {} ready\n", pid);
            break;
        case task::State::S_Running:
            arch::kprint("task {} running\n", pid);
            break;
        case task::State::S_Exited:
            if (WTERMSIG(exitInfo.stat)) {
                arch::kprint("task {} signal with {}\n", pid, WTERMSIG(exitInfo.stat));
            } else {
                arch::kprint("task {} exit with {}\n", pid, WEXITSTATUS(exitInfo.stat));
            }
            break;
        case task::State::S_Blocked:
            switch (blockReason) {
                case BlockReason::BR_Unknown:
                    arch::kprint("task {} blocked\n", pid);
                    break;
                case BlockReason::BR_Sleep:
                    arch::kprint("task {} sleeping, eta {}\n", pid, sleepInfo.time - timer::msSinceBoot);
                    break;
                case BlockReason::BR_WaitInput:
                    arch::kprint("task {} waiting input\n", pid);
                    break;
                case BlockReason::BR_WaitTask:
                    arch::kprint("task {} waiting task {}\n", pid, waitTaskInfo.pid);
                    break;
            }
            break;
    }
}

void TaskControlBlock::sendSignal(int sig) noexcept {
    arch::InterruptGuard guard;
    if (ended()) {
        arch::kprint("signal {} ignored for pid {} as it is ended\n", sig, pid);
        return;
    }
    __signal.__pending_signals |= 1ull << sig;
    if (__signal.isMasked(sig)) {
        arch::kprint("signal {} masked for pid {}\n", sig, pid);
        return;
    }
    if (state == State::S_Blocked) {
        unblock(this, WakeReason::WR_Signal);
    }
}

static void defaultSignalLogic(int sig) {
    if (isSignalDefaultIgnore(sig)) {
        return;
    }
    exitTask(255, sig);
}

bool TaskControlBlock::checkSignal(interrupt::SyscallFrame* frame) noexcept {
    if (this != __scheduler->__current) {
        arch::kfatal("checkSignal should be called with currentTask");
    }

    // TODO: 假设一定来自用户态. 需要有个地方检查frame->cs
    arch::InterruptGuard guard;
    SigSet sigs = __signal.restSignals();
    if (!sigs) {
        return false;
    }
    auto sig = std::countr_zero(sigs);
    __signal.__pending_signals ^= 1ull << sig;
    if (!__signal.__signal_actions) {
        defaultSignalLogic(sig);
    } else {
        const auto& entry = __signal.__signal_actions->operator[](sig);
        if (entry.__handler == SIG_IGN) {
            ;
        } else if (entry.__handler == SIG_DFL) {
            defaultSignalLogic(sig);
        } else {
            auto mask = __signal.__signal_mask;
            __signal.__signal_mask |= entry.__mask;

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

}  // namespace nyan::task
