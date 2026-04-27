#include "task.hpp"

#include <elf.h>
#include <bit>

#include "../allocator/alloc.hpp"
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

lib::List<TaskControlBlockTag, true> pendingTasks;
lib::List<TaskControlBlockTag, true> sleepTasks;

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

    currentTask->childTasks.push_back(tcb);

    return tcb;
}

static int elfEntry(void* param) {
    uint32_t* args = static_cast<uint32_t*>(param);
    jumpRing3(args[0], args[1]);
    return 0;
}

struct LoadElfResult {
    paging::UserDirectory pageDir;
    paging::VirtualAddress brkAddr;
    paging::VirtualAddress entry;
};

static LoadElfResult loadElf(uint8_t* file, size_t) {
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

    return {std::move(pageDir), brkAddr, paging::VirtualAddress{header->entry_offset}};
}

static void loadArgv(Stack& stack, const char* const* argv) {
    lib::vector<paging::VirtualAddress> args;
    for (auto arg = argv; *arg; arg++) {
        args.push_back(stack.translator.toUser(stack.pushString(*arg)));
    }

    // auxv
    stack.pushVal(0);
    stack.pushVal(AT_NULL);
    stack.pushVal(4096);
    stack.pushVal(AT_PAGESZ);

    // envp
    stack.pushVal(0);

    // argv
    stack.pushVal(0);
    for (auto it = args.rbegin(); it != args.rend(); it++) {
        stack.pushVal(it->addr);
    }
    stack.pushVal(stack.userEsp().addr);
    stack.pushVal(args.size());
}

TaskControlBlock* createElfTask(uint8_t* file, size_t size, const char* const* argv) {
    auto [pageDir, brkAddr, entry] = loadElf(file, size);

    Stack stack(pageDir, 0xC0000000_va);
    loadArgv(stack, argv);
    stack.pushVal(stack.userEsp().addr);
    stack.pushVal(entry.addr);

    Stack kernelStack;
    fillStack(kernelStack, elfEntry, stack.userEsp().as<void>());

    auto tcb = allocator::allocAs<TaskControlBlock>();
    tcb->userEsp = kernelStack.esp().addr;
    tcb->cr3 = pageDir.mapper.paddr;
    tcb->kernelEsp = kernelStack.userBase.nextPage().addr;
    tcb->state = State::S_Ready;
    tcb->pid = KP_Invalid;

    tcb->parentPid = currentTask->pid;
    tcb->groupPid = currentTask->groupPid;

    tcb->name = lib::format("elf_{}", argv[0] ? argv[0] : "unknown");
    tcb->brkAddr = brkAddr;
    tcb->pages.push_back(kernelStack.userBase.addr);

    currentTask->childTasks.push_back(tcb);

    return tcb;
}

void execTask(uint8_t* file, size_t size, const char* const* argv, interrupt::SyscallFrame* frame) {
    auto [pageDir, brkAddr, entry] = loadElf(file, size);

    auto tcb = currentTask;

    Stack stack(pageDir, 0xC0000000_va);
    loadArgv(stack, argv);
    lib::string name = lib::format("elf_{}", argv[0] ? argv[0] : "unknown");
    argv = nullptr;

    if (auto oldCr3 = tcb->cr3; oldCr3 != paging::kernelPageDirectory.cr3()) {
        auto oldPageDir = paging::UserDirectory::from(oldCr3);
        oldPageDir.free();
    } else {
        arch::kprint("perform exec on system task!\n");
    }

    tcb->userEsp = stack.userEsp().addr;
    tcb->cr3 = pageDir.mapper.paddr;
    // tcb->kernelEsp
    // tcb->state
    // tcb->pid

    // tcb->parentPid
    // tcb->groupPid

    // tcb->pendingSignals
    // tcb->signalMask
    if (tcb->signalActions) {
        for (auto& entry : *tcb->signalActions) {
            if (entry.__handler != SIG_IGN && entry.__handler != SIG_DFL) {
                entry.__handler = SIG_DFL;
                entry.__mask = 0;
                entry.__flags = 0;
            }
        }
    }

    // fdTable close-on-exec
    // tty
    tcb->tls = {};

    tcb->name = name;
    tcb->brkAddr = brkAddr;
    // tcb->pages

    pageDir.mapper.paddr.setCr3();
    gdt::setTls(tcb->tls);

    frame->eip = entry.addr;
    frame->user_esp = tcb->userEsp;
    frame->eax = 0;
    frame->ebx = 0;
    frame->ecx = 0;
    frame->edx = 0;
    frame->esi = 0;
    frame->edi = 0;
    frame->ebp = 0;
    frame->cs = gdt::userCs;
    frame->user_ss = gdt::userDs;
    frame->user_ds = gdt::userDs;
    frame->user_es = gdt::userDs;
    frame->user_fs = gdt::userDs;
    frame->user_gs = gdt::userDs;
}

static int forkEntry(void* param) {
    arch::cli();
    syscallReturn(param);
    return 0;
}

pid_t forkTask(interrupt::SyscallFrame* frame) {
    auto currPageDir = paging::UserDirectory::from(currentTask->cr3);
    auto newPageDir = paging::UserDirectory::forkCOW(currPageDir);
    currentTask->cr3.setCr3();

    Stack kernelStack;
    auto newFrame = kernelStack.pushAny(*frame);
    newFrame->eax = 0;
    fillStack(kernelStack, forkEntry, kernelStack.esp().as<void>());

    auto tcb = allocator::allocAs<TaskControlBlock>();
    tcb->userEsp = kernelStack.esp().addr;
    tcb->cr3 = newPageDir.mapper.paddr;
    tcb->kernelEsp = kernelStack.userBase.nextPage().addr;
    tcb->state = State::S_Ready;
    tcb->pid = KP_Invalid;

    tcb->parentPid = currentTask->pid;
    tcb->groupPid = currentTask->groupPid;

    tcb->pendingSignals = 0;
    tcb->signalMask = currentTask->signalMask;
    if (currentTask->signalActions) {
        tcb->signalActions.reset(allocator::allocAs<std::array<SigAction, NSIG>>(*currentTask->signalActions));
    }

    tcb->fdTable = currentTask->fdTable;

    tcb->tty = currentTask->tty;

    tcb->tls = currentTask->tls;

    tcb->name = currentTask->name;
    tcb->brkAddr = currentTask->brkAddr;
    tcb->pages.push_back(kernelStack.userBase.addr);

    currentTask->childTasks.push_back(tcb);

    auto pid = addTask(tcb);
    if (pid == KP_Invalid) {
        arch::kfatal("fork failed");
    }
    return pid;
}

pid_t addTask(TaskControlBlock* task) {
    arch::InterruptGuard guard;
    pendingTasks.push_back(task);
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
    if (!currentTask->childTasks.empty()) {
        allTasks[KP_Init]->childTasks.splice(allTasks[KP_Init]->childTasks.end(), currentTask->childTasks);
        sendSignal(allTasks[KP_Init], SIGCHLD);
    }
    if (auto parent = findTask(currentTask->parentPid)) {
        sendSignal(parent, SIGCHLD);
    }
    if (!pendingTasks.empty()) {
        auto task = pendingTasks.front();
        pendingTasks.pop_front();
        switchToTask(task);
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
    parentTask->childTasks.erase({task});

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
    if (!pendingTasks.empty()) {
        auto next = pendingTasks.front();
        pendingTasks.pop_front();
        if (currentTask->state == State::S_Running) {
            currentTask->state = State::S_Ready;
            if (currentTask->pid != KP_Idle) {
                pendingTasks.push_back(currentTask);
            }
        }
        switchToTask(next);
        currentTask->state = State::S_Running;
        gdt::setTls(currentTask->tls);
    } else if (currentTask->state != State::S_Running) {
        switchToTask(allTasks[KP_Idle]);
        currentTask->state = State::S_Running;
        gdt::setTls(currentTask->tls);
    }
}

WakeReason block(BlockReason reason) {
    arch::InterruptGuard guard;
    currentTask->state = State::S_Blocked;
    currentTask->blockReason = reason;
    currentTask->wakeReason = WakeReason::WR_Normal;
    yield();
    auto wr = currentTask->wakeReason;
    currentTask->wakeReason = WakeReason::WR_Normal;
    return wr;
}

void unblock(TaskControlBlock* task, WakeReason reason) {
    if (task->state != State::S_Blocked) {
        return;
    }
    task->state = State::S_Ready;
    task->blockReason = BlockReason::BR_Unknown;
    task->wakeReason = reason;
    if (auto func = std::move(task->requestDetach)) {
        task->requestDetach.reset();
        func(task);
    }

    {
        arch::InterruptGuard guard;
        pendingTasks.push_back(task);
    }
}

WakeReason sleep(uint64_t ms, uint64_t* rest) {
    auto currTs = timer::msSinceBoot + ms;

    arch::InterruptGuard guard;
    currentTask->sleepInfo.time = currTs;

    auto pos = std::find_if(sleepTasks.begin(), sleepTasks.end(),
                            [&](const auto& tcb) { return currTs <= tcb.sleepInfo.time; });
    sleepTasks.insert(pos, currentTask);
    currentTask->requestDetach = +[](TaskControlBlock* task) { sleepTasks.erase({task}); };
    auto reason = block(BlockReason::BR_Sleep);
    if (rest) {
        if (currTs <= timer::msSinceBoot) {
            *rest = 0;
        } else {
            *rest = timer::msSinceBoot - currTs;
        }
    }
    return reason;
}

void checkSleep(interrupt::SyscallFrame* frame) {
    arch::InterruptGuard guard;
    while (!sleepTasks.empty()) {
        auto task = sleepTasks.front();
        if (task->sleepInfo.time < timer::msSinceBoot) {
            sleepTasks.pop_front();
            task->requestDetach.reset();
            unblock(task, WakeReason::WR_Normal);
        } else {
            break;
        }
    }
    if ((timer::msSinceBoot % 10 == 0) && currentTask) {
        if (gdt::isRing3(frame->cs)) {
            checkSignal(frame);
        }
        yield();
    }
}

bool isSignalDefaultIgnore(int sig) {
    return sig == SIGCHLD || sig == SIGURG || sig == SIGWINCH;
}

void sendSignal(TaskControlBlock* task, int sig) {
    arch::InterruptGuard guard;
    if (task->ended()) {
        arch::kprint("signal {} ignored for pid {} as it is ended\n", sig, task->pid);
        return;
    }
    task->pendingSignals |= 1ull << sig;
    if (task->signalMask & (1ull << sig)) {
        arch::kprint("signal {} masked for pid {}\n", sig, task->pid);
        return;
    }
    if (task->state == State::S_Blocked) {
        unblock(task, WakeReason::WR_Signal);
    }
}

void defaultSignalLogic(int sig) {
    if (isSignalDefaultIgnore(sig)) {
        return;
    }
    exitTask(255, sig);
}

bool peekSignal() {
    SigSet sigs = currentTask->pendingSignals & ~currentTask->signalMask;
    if (!sigs) {
        return false;
    }

    while (sigs) {
        auto sig = std::countr_zero(sigs);
        sigs ^= 1ull << sig;
        if (!currentTask->signalActions) {
            if (!isSignalDefaultIgnore(sig)) {
                return true;
            }
        } else {
            const auto& entry = currentTask->signalActions->operator[](sig);
            if (entry.__handler == SIG_IGN) {
                continue;
            } else if (entry.__handler == SIG_DFL) {
                if (!isSignalDefaultIgnore(sig)) {
                    return true;
                }
            } else {
                return true;
            }
        }
    }
    return false;
}

bool checkSignal(interrupt::SyscallFrame* frame) {
    // TODO: 假设一定来自用户态. 需要有个地方检查frame->cs
    arch::InterruptGuard guard;
    SigSet sigs = currentTask->pendingSignals & ~currentTask->signalMask;
    if (!sigs) {
        return false;
    }
    auto sig = std::countr_zero(sigs);
    currentTask->pendingSignals ^= 1ull << sig;
    if (!currentTask->signalActions) {
        defaultSignalLogic(sig);
    } else {
        const auto& entry = currentTask->signalActions->operator[](sig);
        if (entry.__handler == SIG_IGN) {
            ;
        } else if (entry.__handler == SIG_DFL) {
            defaultSignalLogic(sig);
        } else {
            auto mask = currentTask->signalMask;
            currentTask->signalMask |= entry.__mask;

            // TODO: 这里关闭了中断, 需要检查是否跨页了.
            auto esp = frame->user_esp;
            esp -= sizeof(task::SignalFrame);
            auto userFrame = reinterpret_cast<task::SignalFrame*>(esp);
            userFrame->retAddr = (0xFFFFF000_va + (sigreturn_trampoline - trampoline_start)).addr;
            userFrame->signal = sig;
            userFrame->oldMask = mask;
            userFrame->frame = *frame;

            frame->eip = reinterpret_cast<uint32_t>(entry.__handler);
            frame->user_esp = esp;
            return true;
        }
    }
    return true;
}

}  // namespace nyan::task
