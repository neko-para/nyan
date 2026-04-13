#include "task.hpp"

#include "../allocator/load.hpp"
#include "../allocator/pool.hpp"
#include "../allocator/utils.hpp"
#include "../elf/entry.hpp"
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

    exitTask(code);
}

uint32_t makeStack(int (*func)(void* param), void* param, uint32_t stackBase) {
    auto base = reinterpret_cast<uint32_t*>(stackBase);
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
    uint32_t stack = reinterpret_cast<uint32_t>(allocator::frameAlloc());
    auto tcb = allocator::allocAs<TaskControlBlock>();
    tcb->userEsp = makeStack(func, param, stack);
    tcb->cr3 = paging::kernelPageDirectory.cr3();
    tcb->kernelEsp = stack + (1 << 10);
    tcb->state = State::S_Ready;
    tcb->pid = KP_Invalid;

    tcb->pages.push_back(stack);

    return tcb;
}

static int elfEntry(void* entry) {
    jumpRing3(reinterpret_cast<void (*)()>(entry));
    return 0;
}

TaskControlBlock* createElfTask(uint8_t* file, size_t) {
    auto header = new (file) elf::Header;
    // TODO: check if support

    auto pageDir = paging::kernelPageDirectory.fork();

    auto offset = header->program_header_table_offset;
    for (size_t i = 0; i < header->program_header_entry_count; i++) {
        auto program_header = new (file + offset) elf::ProgramHeader;
        offset += header->program_header_entry_size;
        if (program_header->type != elf::PHT_Load) {
            continue;
        }
        if (program_header->align != 0x1000) {
            continue;
        }
        uint32_t lower = program_header->vaddr;
        uint32_t upper = program_header->vaddr + program_header->memsz;
        uint32_t lowerPage = lower & (~0xFFF);
        uint32_t upperPage = (upper + 0xFFF) & (~0xFFF);
        for (uint32_t vaddr = lowerPage; vaddr != upperPage; vaddr += 0x1000) {
            auto physicalOffset = allocator::poolManager->alloc();
            auto physicalAddr = allocator::PoolManager::pageAt(physicalOffset);

            auto tableLocation = vaddr >> 22;
            if (!pageDir->at(tableLocation)) {
                auto table = allocator::frameAllocAs<paging::Table>();
                pageDir->set(virtualToPhysical(table), tableLocation,
                             paging::PDE_Present | paging::PDE_ReadWrite | paging::PDE_User);
            }
            auto attr = paging::PTE_Present | paging::PTE_User;
            if (program_header->flags & elf::PHF_Writable) {
                attr |= paging::PTE_ReadWrite;
            }
            pageDir->map(physicalAddr, vaddr, attr);

            // TODO: map to upper to fill data
        }
    }

    uint32_t kernelStack = reinterpret_cast<uint32_t>(allocator::frameAlloc());
    uint32_t userStack = 0xC0000000 - 0x1000;

    {
        auto physicalOffset = allocator::poolManager->alloc();
        auto physicalAddr = allocator::PoolManager::pageAt(physicalOffset);

        auto tableLocation = userStack >> 22;
        if (!pageDir->at(tableLocation)) {
            auto table = allocator::frameAllocAs<paging::Table>();
            pageDir->set(virtualToPhysical(table), tableLocation,
                         paging::PDE_Present | paging::PDE_ReadWrite | paging::PDE_User);
        }
        pageDir->map(physicalAddr, userStack, paging::PTE_Present | paging::PTE_ReadWrite | paging::PTE_User);
    }

    // TODO: fix page fault. stack should be map to upper to fill data

    auto tcb = allocator::allocAs<TaskControlBlock>();
    tcb->userEsp = makeStack(elfEntry, reinterpret_cast<void*>(header->entry_offset), userStack);
    tcb->cr3 = pageDir->cr3();
    tcb->kernelEsp = kernelStack + 0x1000;
    tcb->state = State::S_Ready;
    tcb->pid = KP_Invalid;

    tcb->pages.push_back(kernelStack);

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

[[noreturn]] void exitTask(int code) {
    arch::cli();
    currentTask->state = State::S_Exited;
    currentTask->exitInfo.code = code;
    aliveTaskCount--;
    if (pendingTasks) {
        switchToTask(pendingTasks.popFront());
    } else if (aliveTaskCount == 0) {
        switchToTask(allTasks[KP_Init]);
    } else {
        switchToTask(allTasks[KP_Idle]);
    }
    arch::kfatal("exited task rescheduled!");
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
            if (currentTask != KP_Idle) {
                pendingTasks.pushBack(currentTask.head);
            }
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
