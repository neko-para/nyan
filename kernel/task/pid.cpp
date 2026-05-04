#include "pid.hpp"

#include <algorithm>

#include "../arch/guard.hpp"
#include "../paging/directory.hpp"
#include "scheduler.hpp"
#include "task.hpp"
#include "tcb.hpp"

namespace nyan::task {

TaskControlBlock* __all_tasks[__max_task];

static int idleTask(void*) {
    while (true) {
        arch::hlt();
        __scheduler->yield();
    }
    return 0;
}

pid_t allocPid(TaskControlBlock* task) {
    arch::InterruptGuard guard;
    for (pid_t p = KP_FirstUser; p < __max_task; p++) {
        if (!__all_tasks[p]) {
            __all_tasks[p] = task;
            task->pid = p;
            return p;
        }
    }
    return KP_Invalid;
}

void setupKnownTasks() {
    std::fill_n(__all_tasks, __max_task, nullptr);

    TaskControlBlock* initTask = new TaskControlBlock;
    initTask->cr3 = paging::kernelPageDirectory.cr3();
    initTask->state = State::S_Running;
    initTask->pid = KP_Init;
    initTask->name = "init";
    initTask->parentPid = KP_Invalid;
    initTask->groupPid = KP_Init;
    __all_tasks[KP_Init] = initTask;
    __scheduler->__current = initTask;

    auto task = createTask(idleTask);
    task->parentPid = KP_Invalid;
    task->groupPid = KP_Idle;
    task->pid = KP_Idle;
    task->name = "idle";
    __all_tasks[KP_Idle] = task;

    initTask->childTasks.erase({task});
}

TaskControlBlock* findTask(pid_t pid) {
    if (pid < 0 || pid >= __max_task) {
        return nullptr;
    }
    return __all_tasks[pid];
}

}  // namespace nyan::task
