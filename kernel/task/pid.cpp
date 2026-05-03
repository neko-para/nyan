#include "pid.hpp"

#include <algorithm>

#include "../arch/guard.hpp"
#include "../paging/directory.hpp"
#include "task.hpp"
#include "tcb.hpp"

namespace nyan::task {

TaskControlBlock* allTasks[__max_task];

static int idleTask(void*) {
    while (true) {
        arch::hlt();
        task::yield();
    }
    return 0;
}

pid_t allocPid(TaskControlBlock* task) {
    arch::InterruptGuard guard;
    for (pid_t p = KP_FirstUser; p < __max_task; p++) {
        if (!allTasks[p]) {
            allTasks[p] = task;
            task->pid = p;
            return p;
        }
    }
    return KP_Invalid;
}

void setupKnownTasks() {
    std::fill_n(allTasks, __max_task, nullptr);

    TaskControlBlock* initTask = new TaskControlBlock;
    initTask->cr3 = paging::kernelPageDirectory.cr3();
    initTask->state = State::S_Running;
    initTask->pid = KP_Init;
    initTask->name = "init";
    initTask->parentPid = KP_Invalid;
    initTask->groupPid = KP_Init;
    currentTask = {initTask};
    allTasks[KP_Init] = initTask;

    auto task = createTask(idleTask);
    task->parentPid = KP_Invalid;
    task->groupPid = KP_Idle;
    task->pid = KP_Idle;
    task->name = "idle";
    allTasks[KP_Idle] = task;

    initTask->childTasks.erase({task});
}

TaskControlBlock* findTask(pid_t pid) {
    if (pid < 0 || pid >= __max_task) {
        return nullptr;
    }
    return allTasks[pid];
}

}  // namespace nyan::task
