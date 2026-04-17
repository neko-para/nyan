#include "pid.hpp"

#include <algorithm>

#include "guard.hpp"
#include "task.hpp"
#include "tcb.hpp"

namespace nyan::task {

TaskControlBlock* allTasks[MaxTaskCount];

static int idleTask(void*) {
    while (true) {
        arch::hlt();
        task::yield();
    }
    return 0;
}

pid_t allocPid(TaskControlBlock* task) {
    InterruptGuard guard;
    for (pid_t p = KP_FirstUser; p < MaxTaskCount; p++) {
        if (!allTasks[p]) {
            allTasks[p] = task;
            task->pid = p;
            return p;
        }
    }
    return KP_Invalid;
}

void setupKnownTasks() {
    std::fill_n(allTasks, MaxTaskCount, nullptr);

    auto task = createTask(idleTask);
    task->pid = KP_Idle;
    allTasks[KP_Idle] = task;
}

TaskControlBlock* findTask(pid_t pid) {
    if (pid < 0 || pid >= MaxTaskCount) {
        return nullptr;
    }
    return allTasks[pid];
}

}  // namespace nyan::task
