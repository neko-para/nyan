#include "tcb.hpp"

#include "../timer/load.hpp"
#include "../vga/print.hpp"
#include "task.hpp"

namespace nyan::task {

lib::List<TaskControlBlock> currentTask asm("currentTask");

void TaskControlBlock::dump() {
    switch (state) {
        case State::S_Ready:
            vga::print("task {} ready\n", pid);
            break;
        case task::State::S_Running:
            vga::print("task {} running\n", pid);
            break;
        case task::State::S_Exited:
            vga::print("task {} exited with {}\n", pid, exitInfo.code);
            break;
        case task::State::S_Blocked:
            switch (blockReason) {
                case BlockReason::BR_Unknown:
                    vga::print("task {} blocked\n", pid);
                    break;
                case BlockReason::BR_Sleep:
                    vga::print("task {} sleeping, eta {}\n", pid, sleepInfo.time - timer::msSinceBoot);
                    break;
                case BlockReason::BR_WaitInput:
                    vga::print("task {} waiting input\n", pid);
                    break;
                case BlockReason::BR_WaitTask:
                    vga::print("task {} waiting task {}\n", pid, waitInfo.pid);
                    break;
            }
            break;
    }
}

}  // namespace nyan::task
