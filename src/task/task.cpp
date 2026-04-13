#include "task.hpp"

#include "../allocator/utils.hpp"
#include "../paging/kernel.hpp"
#include "../timer/load.hpp"
#include "../vga/print.hpp"
#include "guard.hpp"

namespace nyan::task {

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
            }
            break;
    }
}

lib::List<TaskControlBlock> currentTask asm("currentTask");
lib::TailList<TaskControlBlock> pendingTasks;
lib::TailList<TaskControlBlock> sleepTasks;

uint32_t aliveTaskCount = 0;

void load() {
    setupKnownTasks();
}

__attribute__((noinline)) void taskWrapper(int (*func)(void* param), void* param) {
    currentTask->state = State::S_Running;
    if (currentTask->pid != KP_Idle) {
        aliveTaskCount++;
    }
    arch::sti();

    auto code = func(param);

    arch::cli();
    currentTask->state = State::S_Exited;
    currentTask->exitInfo.code = code;
    if (--aliveTaskCount == 0) {
        switchToTask(allTasks[KP_Init]);
    } else if (pendingTasks) {
        switchToTask(pendingTasks.popFront());
    } else {
        switchToTask(allTasks[KP_Idle]);
    }
}

uint32_t makeStack(int (*func)(void* param), void* param, uint32_t& stackBase) {
    auto base = static_cast<uint32_t*>(allocator::frameAlloc());
    stackBase = reinterpret_cast<uint32_t>(base);
    auto stack = base + (1 << 10);  // +4K
    *--stack = reinterpret_cast<uint32_t>(param);
    *--stack = reinterpret_cast<uint32_t>(func);
    *--stack = 0x12345678;  // fake eip
    *--stack = reinterpret_cast<uint32_t>(taskWrapper);
    *--stack = 0x2;  // flags
    *--stack = 0;    // ebx
    *--stack = 0;    // esi
    *--stack = 0;    // edi
    *--stack = 0;    // ebp
    return reinterpret_cast<uint32_t>(stack);
}

TaskControlBlock* createTask(int (*func)(void* param), void* param) {
    uint32_t stack;
    auto tcb = allocator::allocAs<TaskControlBlock>();
    tcb->userEsp = makeStack(func, param, stack);
    tcb->cr3 = paging::kernelPageDirectory.cr3();
    tcb->kernelEsp = stack + (1 << 10);
    tcb->state = State::S_Ready;
    tcb->pid = KP_Invalid;

    tcb->pages.push_back(stack);

    return tcb;
}

pid_t addTask(TaskControlBlock* task) {
    InterruptGuard guard;
    pendingTasks.pushBack(task);
    if (task->pid == KP_Invalid) {
        return allocPid(task);
    } else {
        return task->pid;
    }
}

__attribute__((noinline)) void initYield() {
    TaskControlBlock* self = allocator::allocAs<TaskControlBlock>();
    self->cr3 = paging::kernelPageDirectory.cr3();
    self->pid = KP_Init;
    self->state = State::S_Blocked;
    currentTask.pushFront(self);
    allTasks[KP_Init] = self;

    auto task = pendingTasks.popFront();
    switchToTask(task);
    self->state = State::S_Running;
}

pid_t runTask(int (*func)(void* param), void* param) {
    auto task = createTask(func, param);
    return addTask(task);
}

bool freeTask(pid_t pid, int* code) {
    auto task = allTasks[pid];
    if (!task) {
        vga::print("Task {} not exists!\n", pid);
        return false;
    }
    if (task->state != State::S_Exited) {
        vga::print("Task {} not exited!\n", pid);
        return false;
    }
    if (code) {
        *code = task->exitInfo.code;
    }

    for (auto page : task->pages) {
        allocator::frameFree(reinterpret_cast<void*>(page));
    }
    allocator::freeAs(task);
    allTasks[pid] = nullptr;

    return true;
}

__attribute__((noinline)) void yield() {
    InterruptGuard guard;
    if (pendingTasks) {
        auto next = pendingTasks.popFront();
        if (currentTask->state == State::S_Running) {
            currentTask->state = State::S_Ready;
            pendingTasks.pushBack(currentTask.head);
        }
        switchToTask(next);
        currentTask->state = State::S_Running;
    } else if (currentTask->state != State::S_Running) {
        switchToTask(allTasks[KP_Idle]);
        currentTask->state = State::S_Running;
    }
}

void block(BlockReason reason) {
    InterruptGuard guard;
    currentTask->state = State::S_Blocked;
    currentTask->blockReason = reason;
    yield();
}

void unblock(TaskControlBlock* task) {
    if (task->state != State::S_Blocked) {
        return;
    }
    task->state = State::S_Ready;
    task->blockReason = BlockReason::BR_Unknown;

    {
        InterruptGuard guard;
        pendingTasks.pushBack(task);
    }
}

void sleep(uint64_t ms) {
    auto currTs = timer::msSinceBoot + ms;

    InterruptGuard guard;
    currentTask->state = State::S_Blocked;
    currentTask->blockReason = BlockReason::BR_Sleep;
    currentTask->sleepInfo.time = currTs;

    if (!sleepTasks.head) {
        sleepTasks.pushBack(currentTask.head);
    } else if (currTs <= sleepTasks.head->sleepInfo.time) {
        currentTask.pushFront(currentTask.head);
    } else {
        for (auto head = sleepTasks.head; head->ListNode<TaskControlBlockTag>::next;
             head = head->ListNode<TaskControlBlockTag>::next) {
            auto next = head->ListNode<TaskControlBlockTag>::next;
            if (currTs <= next->sleepInfo.time) {
                head->ListNode<TaskControlBlockTag>::next = currentTask.head;
                currentTask->ListNode<TaskControlBlockTag>::next = next;
                goto inserted;
            }
        }
        sleepTasks.pushBack(currentTask.head);
    inserted:;
    }

    yield();
}

void checkSleep() {
    InterruptGuard guard;
    while (sleepTasks && sleepTasks.head->sleepInfo.time < timer::msSinceBoot) {
        auto task = sleepTasks.popFront();
        pendingTasks.pushBack(task);
    }
    if (timer::msSinceBoot % 10 == 0 && currentTask) {
        yield();
    }
}

}  // namespace nyan::task
