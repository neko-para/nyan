#include "tcb.hpp"

#include "../console/entry.hpp"
#include "../timer/load.hpp"
#include "task.hpp"

namespace nyan::task {

lib::List<TaskControlBlockTag> currentTask asm("currentTask");

void TaskControlBlock::dump() {
    switch (state) {
        case State::S_Ready:
            console::activeTty->print("task {} ready\n", pid);
            break;
        case task::State::S_Running:
            console::activeTty->print("task {} running\n", pid);
            break;
        case task::State::S_Exited:
            console::activeTty->print("task {} exited with {}\n", pid, exitInfo.code);
            break;
        case task::State::S_Blocked:
            switch (blockReason) {
                case BlockReason::BR_Unknown:
                    console::activeTty->print("task {} blocked\n", pid);
                    break;
                case BlockReason::BR_Sleep:
                    console::activeTty->print("task {} sleeping, eta {}\n", pid, sleepInfo.time - timer::msSinceBoot);
                    break;
                case BlockReason::BR_WaitInput:
                    console::activeTty->print("task {} waiting input\n", pid);
                    break;
                case BlockReason::BR_WaitTask:
                    console::activeTty->print("task {} waiting task {}\n", pid, waitInfo.pid);
                    break;
            }
            break;
    }
}

}  // namespace nyan::task
