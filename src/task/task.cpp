#include "task.hpp"

#include <bit>

#include "../allocator/utils.hpp"
#include "../arch/guard.hpp"
#include "../elf/entry.hpp"
#include "../gdt/load.hpp"
#include "../paging/directory.hpp"
#include "../paging/translator.hpp"
#include "../timer/load.hpp"
#include "signal.hpp"
#include "stack.hpp"
#include "switch.hpp"
#include "tcb.hpp"
#include "trampoline.h"
#include "wait.hpp"

namespace nyan::task {

lib::TailList<TaskControlBlockTag> pendingTasks;
lib::TailList<TaskControlBlockTag> sleepTasks;

void loadTrampoline() {
    paging::kernelPageDirectory.set(paging::kernelPageDirectory.at(1023), 1023,
                                    paging::PDE_Present | paging::PDE_ReadWrite | paging::PDE_User);
    paging::kernelPageDirectory.map(0xFFFFF000_va, allocator::physicalFrameAlloc(),
                                    paging::PTE_Present | paging::PTE_ReadWrite | paging::PTE_User);
    0xFFFFF000_va .invlpg();
    void* frame = 0xFFFFF000_va .as<void*>();
    std::memset(frame, 0, 4096);
    std::memcpy(frame, trampoline_start, trampoline_end - trampoline_start);
    // TODO: 这里可以配置完成后把readwrite干掉
}

void load() {
    setupKnownTasks();
    loadTrampoline();
}

__attribute__((noinline)) void taskWrapper(int (*func)(void* param), void* param) {
    currentTask->state = State::S_Running;
    arch::sti();

    auto code = func(param);

    exitTask(code);
}

void fillStack(Stack& stack, int (*func)(void* param), void* param) {
    stack.pushPtr(param);
    stack.pushPtr(func);
    stack.pushVal(0x12345678);   // fake eip
    stack.pushPtr(taskWrapper);  // entry
    stack.pushVal(0x2);          // flags
    stack.pushVal(0);            // ebx
    stack.pushVal(0);            // esi
    stack.pushVal(0);            // edi
    stack.pushVal(0);            // ebp
}

TaskControlBlock* createTask(int (*func)(void* param), void* param) {
    Stack kernelStack;
    Stack stack;
    fillStack(stack, func, param);
    auto tcb = allocator::allocAs<TaskControlBlock>();
    tcb->userEsp = stack.esp().addr;
    tcb->cr3 = paging::kernelPageDirectory.cr3();
    tcb->kernelEsp = kernelStack.esp().addr;
    tcb->state = State::S_Ready;
    tcb->pid = KP_Invalid;

    tcb->parentPid = currentTask->pid;
    tcb->groupPid = currentTask->groupPid;
    tcb->brkAddr = 0x400000_va;
    tcb->pages.push_back(kernelStack.userBase.addr);
    tcb->pages.push_back(stack.userBase.addr);

    currentTask->childTasks.pushBack(tcb);

    return tcb;
}

static int elfEntry(void* param) {
    uint32_t* args = static_cast<uint32_t*>(param);
    jumpRing3(args[0], args[1], args + 2);
    return 0;
}

TaskControlBlock* createElfTask(uint8_t* file, size_t, const char* const* argv) {
    auto header = new (file) elf::Header;
    // TODO: check if support

    auto pageDir = paging::UserDirectory::fork(paging::kernelPageDirectory);

    auto offset = header->program_header_table_offset;
    auto brkAddr = 0x400000_va;
    for (size_t i = 0; i < header->program_header_entry_count; i++) {
        auto program_header = new (file + offset) elf::ProgramHeader;
        offset += header->program_header_entry_size;
        if (program_header->type != elf::PHT_Load) {
            continue;
        }
        if (program_header->align != 0x1000) {
            continue;
        }
        auto lower = paging::VirtualAddress{program_header->vaddr};
        auto upper = paging::VirtualAddress{program_header->vaddr + program_header->memsz};
        auto fileUpper = paging::VirtualAddress{program_header->vaddr + program_header->filesz};
        auto lowerPage = lower.alignDown();
        auto upperPage = upper.alignUp();
        brkAddr = std::max(brkAddr, upperPage);
        for (auto vaddr = lowerPage; vaddr != upperPage; vaddr = vaddr.nextPage()) {
            auto mapper = pageDir.alloc(vaddr, program_header->flags & elf::PHF_Writable);
            auto frame = mapper.as<uint8_t>();

            auto lower_bound = std::max(lower, vaddr);
            auto upper_bound = std::min(fileUpper, vaddr.nextPage());
            if (lower_bound < upper_bound) {
                std::copy_n(&file[program_header->offset + (lower_bound - lower)], upper_bound - lower_bound,
                            &frame[lower_bound - vaddr]);
            }
        }
    }

    Stack kernelStack;
    Stack stack(pageDir, 0xC0000000_va);
    lib::vector<paging::VirtualAddress> args;
    for (auto arg = argv; *arg; arg++) {
        args.push_back(stack.translator.toUser(stack.pushString(*arg)));
    }
    stack.pushVal(0);
    for (auto it = args.rbegin(); it != args.rend(); it++) {
        stack.pushVal(it->addr);
    }
    stack.pushVal(args.size());
    stack.pushVal(header->entry_offset);
    auto argPtr = stack.userEsp();
    fillStack(stack, elfEntry, argPtr.as<void>());

    auto tcb = allocator::allocAs<TaskControlBlock>();
    tcb->userEsp = stack.userEsp().addr;
    tcb->cr3 = pageDir.mapper.paddr;
    tcb->kernelEsp = kernelStack.esp().addr;
    tcb->state = State::S_Ready;
    tcb->pid = KP_Invalid;

    tcb->parentPid = currentTask->pid;
    tcb->groupPid = currentTask->groupPid;
    tcb->name = lib::format("elf_{}", argv[0] ? argv[0] : "unknown");
    tcb->brkAddr = brkAddr;
    tcb->pages.push_back(kernelStack.userBase.addr);

    currentTask->childTasks.pushBack(tcb);

    return tcb;
}

pid_t addTask(TaskControlBlock* task) {
    arch::InterruptGuard guard;
    pendingTasks.pushBack(task);
    if (task->pid == KP_Invalid) {
        return allocPid(task);
    } else {
        return task->pid;
    }
}

[[noreturn]] void exitTask(int code, int sig) {
    arch::cli();

    if (currentTask->pid == KP_Init) {
        arch::kfatal("init task cannot exit!");
    }

    currentTask->state = State::S_Exited;
    currentTask->exitInfo.stat = (code << 8) | sig;
    if (currentTask->childTasks) {
        allTasks[KP_Init]->childTasks.appendBack(currentTask->childTasks);
        allTasks[KP_Init]->wait.wakeOne();
    }
    if (auto parent = findTask(currentTask->parentPid)) {
        parent->wait.wakeOne();
    }
    if (pendingTasks) {
        switchToTask(pendingTasks.popFront());
    } else {
        switchToTask(allTasks[KP_Idle]);
    }
    arch::kfatal("exited task rescheduled!");
}

bool freeTask(pid_t pid, int* stat) {
    arch::kprint("free task pid = {} current = {} {}\n", pid, currentTask->pid, currentTask->name);
    auto task = allTasks[pid];
    if (!task) {
        arch::kprint("Task {} not exists!\n", pid);
        return false;
    }
    if (task->state != State::S_Exited) {
        arch::kprint("Task {} not exited!\n", pid);
        return false;
    }
    if (stat) {
        *stat = task->exitInfo.stat;
    }

    auto parentTask = findTask(task->parentPid);
    parentTask->childTasks.take(task);

    for (auto page : task->pages) {
        allocator::frameFree(reinterpret_cast<void*>(page));
    }

    if (task->cr3 != paging::kernelPageDirectory.cr3()) {
        auto userPage = paging::UserDirectory::from(task->cr3);
        userPage.free();
        allocator::physicalFrameRelease(task->cr3);
    }

    allocator::freeAs(task);
    allTasks[pid] = nullptr;

    return true;
}

__attribute__((noinline)) void yield() {
    arch::InterruptGuard guard;
    if (pendingTasks) {
        auto next = pendingTasks.popFront();
        if (currentTask->state == State::S_Running) {
            currentTask->state = State::S_Ready;
            if (currentTask->pid != KP_Idle) {
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
    arch::InterruptGuard guard;
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
        arch::InterruptGuard guard;
        pendingTasks.pushBack(task);
    }
}

void sleep(uint64_t ms) {
    auto currTs = timer::msSinceBoot + ms;

    arch::InterruptGuard guard;
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

void checkSleep(interrupt::SyscallFrame* frame) {
    arch::InterruptGuard guard;
    while (sleepTasks && sleepTasks.head->sleepInfo.time < timer::msSinceBoot) {
        auto task = sleepTasks.popFront();
        pendingTasks.pushBack(task);
    }
    if ((timer::msSinceBoot % 10 == 0) && currentTask) {
        if (gdt::isRing3(frame->cs)) {
            checkSignal(frame);
        }
        yield();
    }
}

void sendSignal(TaskControlBlock* task, int sig) {
    arch::InterruptGuard guard;
    if (task->ended()) {
        arch::kprint("signal {} ignored for pid {} as it is ended\n", sig, task->pid);
        return;
    }
    if (task->signalMask & (1u << sig)) {
        arch::kprint("signal {} masked for pid {}\n", sig, task->pid);
        return;
    }
    task->pendingSignals |= 1u << sig;
    if (task->state == State::S_Blocked) {
        unblock(task);
    }
}

void defaultSignalLogic(int sig) {
    switch (sig) {
        case SIGCHLD:
        case SIGURG:
        case SIGWINCH:
            break;
        default:
            exitTask(255, sig);
            break;
    }
}

bool checkSignal(interrupt::SyscallFrame* frame) {
    // TODO: 假设一定来自用户态. 需要有个地方检查frame->cs
    arch::InterruptGuard guard;
    if (!currentTask->pendingSignals) {
        return false;
    }
    auto sig = std::countr_zero(currentTask->pendingSignals);
    currentTask->pendingSignals &= ~(1u << sig);
    if (!currentTask->signalActions) {
        defaultSignalLogic(sig);
    } else {
        const auto& entry = currentTask->signalActions->operator[](sig);
        if (entry.sa_handler == SIG_IGN) {
            ;
        } else if (entry.sa_handler == SIG_DFL) {
            defaultSignalLogic(sig);
        } else {
            auto mask = currentTask->signalMask;
            currentTask->signalMask |= entry.sa_mask;

            // TODO: 这里关闭了中断, 需要检查是否跨页了.
            auto esp = frame->user_esp;
            esp -= sizeof(task::SignalFrame);
            auto userFrame = reinterpret_cast<task::SignalFrame*>(esp);
            userFrame->retAddr = (0xFFFFF000_va + (sigreturn_trampoline - trampoline_start)).addr;
            userFrame->signal = sig;
            userFrame->oldMask = mask;
            userFrame->frame = *frame;

            frame->eip = reinterpret_cast<uint32_t>(entry.sa_handler);
            frame->user_esp = esp;
            return true;
        }
    }
    return true;
}

}  // namespace nyan::task
