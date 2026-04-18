#include "tcb.hpp"

#include "../timer/load.hpp"
#include "../tty/entry.hpp"
#include "task.hpp"

namespace nyan::task {

lib::List<TaskControlBlock> currentTask asm("currentTask");

void TaskControlBlock::dump() {
    switch (state) {
        case State::S_Ready:
            tty::activeTty->print("task {} ready\n", pid);
            break;
        case task::State::S_Running:
            tty::activeTty->print("task {} running\n", pid);
            break;
        case task::State::S_Exited:
            tty::activeTty->print("task {} exited with {}\n", pid, exitInfo.code);
            break;
        case task::State::S_Blocked:
            switch (blockReason) {
                case BlockReason::BR_Unknown:
                    tty::activeTty->print("task {} blocked\n", pid);
                    break;
                case BlockReason::BR_Sleep:
                    tty::activeTty->print("task {} sleeping, eta {}\n", pid, sleepInfo.time - timer::msSinceBoot);
                    break;
                case BlockReason::BR_WaitInput:
                    tty::activeTty->print("task {} waiting input\n", pid);
                    break;
                case BlockReason::BR_WaitTask:
                    tty::activeTty->print("task {} waiting task {}\n", pid, waitInfo.pid);
                    break;
            }
            break;
    }
}

}  // namespace nyan::task
