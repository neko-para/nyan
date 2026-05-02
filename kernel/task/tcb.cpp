#include "tcb.hpp"

#include <sys/wait.h>

#include "../timer/load.hpp"
#include "task.hpp"

namespace nyan::task {

TaskControlBlock* currentTask asm("currentTask");

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

}  // namespace nyan::task
