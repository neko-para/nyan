#include "task.hpp"

#include "../allocator/utils.hpp"
#include "../paging/kernel.hpp"

namespace nyan::task {

lib::List<TaskControlBlock> currentTask asm("currentTask");
lib::TailList<TaskControlBlock> pendingTasks;
TaskControlBlock* initTask;

void taskWrapper(void (*func)(void* param), void* param) {
    func(param);

    if (pendingTasks) {
        switchToTask(pendingTasks.popFront());
    } else {
        switchToTask(initTask);
    }
}

uint32_t makeStack(void (*func)(void* param), void* param, uint32_t& stackBase) {
    auto base = static_cast<uint32_t*>(allocator::frameAlloc());
    stackBase = reinterpret_cast<uint32_t>(base);
    auto stack = base + (1 << 10);  // +4K
    *--stack = reinterpret_cast<uint32_t>(param);
    *--stack = reinterpret_cast<uint32_t>(func);
    *--stack = 0x12345678;  // fake eip
    *--stack = reinterpret_cast<uint32_t>(taskWrapper);
    *--stack = 0;  // ebp
    *--stack = 0;  // edi
    *--stack = 0;  // esi
    *--stack = 0;  // ebx
    return reinterpret_cast<uint32_t>(stack);
}

TaskControlBlock* createTask(void (*func)(void* param), void* param) {
    uint32_t stack;
    auto tcb = allocator::allocAs<TaskControlBlock>();
    tcb->userEsp = makeStack(func, param, stack);
    tcb->cr3 = paging::kernelPageDirectory.cr3();
    tcb->kernelEsp = stack + (1 << 10);
    return tcb;
}

void addTask(TaskControlBlock* task) {
    pendingTasks.pushBack(task);
}

void initYield() {
    TaskControlBlock self;
    self.cr3 = paging::kernelPageDirectory.cr3();
    currentTask.pushFront(&self);
    initTask = &self;

    auto task = pendingTasks.popFront();
    switchToTask(task);
}

}  // namespace nyan::task
