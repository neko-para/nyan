#include "task.hpp"

#include <elf.h>
#include <vector>

#include "../allocator/mod.hpp"
#include "../arch/guard.hpp"
#include "../elf/entry.hpp"
#include "../fs/load.hpp"
#include "../gdt/load.hpp"
#include "../paging/directory.hpp"
#include "../paging/translator.hpp"
#include "../timer/load.hpp"
#include "pid.hpp"
#include "scheduler.hpp"
#include "stack.hpp"
#include "switch.hpp"
#include "tcb.hpp"

namespace nyan::task {

lib::List<TaskControlBlockTag, true> pendingTasks;
lib::List<TaskControlBlockTag, true> sleepTasks;

__attribute__((noinline)) void taskWrapper(int (*func)(void* param), void* param) {
    __scheduler->__current->state = State::S_Running;
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
    auto tcb = new TaskControlBlock;
    tcb->userEsp = stack.esp().addr;
    tcb->cr3 = paging::kernelPageDirectory.cr3();
    tcb->kernelEsp = kernelStack.esp().addr;
    tcb->state = State::S_Ready;
    tcb->pid = KP_Invalid;

    tcb->parentPid = __scheduler->__current->pid;
    tcb->groupPid = __scheduler->__current->groupPid;
    tcb->brkBase = 0x400000_va;
    tcb->brkAddr = 0x400000_va;
    tcb->stackRange = {0xC0000000_va - 0x800000, 0xC0000000_va};
    tcb->pages.push_back(kernelStack.userBase.addr);
    tcb->pages.push_back(stack.userBase.addr);

    tcb->cwd = fs::rootEntry()->__mount_point;

    __scheduler->__current->childTasks.push_back(tcb);

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

static LoadElfResult loadElf(paging::VMSpace& vmSpace, uint8_t* file, size_t) {
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
        paging::VMA vma;
        vma.__begin = lowerPage;
        vma.__end = upperPage;
        vma.__protect = 0;
        if (program_header->flags & elf::PHF_Executable) {
            vma.__protect |= PROT_EXEC;
        }
        if (program_header->flags & elf::PHF_Writable) {
            vma.__protect |= PROT_WRITE;
        }
        if (program_header->flags & elf::PHF_Readable) {
            vma.__protect |= PROT_READ;
        }
        vma.__flags = MAP_PRIVATE | MAP_ANONYMOUS;
        vma.__name = "elf segs";
        vmSpace.insert(vma);
    }

    return {std::move(pageDir), brkAddr, paging::VirtualAddress{header->entry_offset}};
}

static void loadArgv(Stack& stack, const char* const* argv, const char* const* envp) {
    std::vector<paging::VirtualAddress> args;
    for (auto arg = argv; *arg; arg++) {
        args.push_back(stack.translator.toUser(stack.pushString(*arg)));
    }
    std::vector<paging::VirtualAddress> envs;
    if (envp) {
        for (auto env = envp; *env; env++) {
            envs.push_back(stack.translator.toUser(stack.pushString(*env)));
        }
    }

    // auxv
    stack.pushVal(0);
    stack.pushVal(AT_NULL);
    stack.pushVal(4096);
    stack.pushVal(AT_PAGESZ);

    // envp
    stack.pushVal(0);
    for (auto it = envs.rbegin(); it != envs.rend(); it++) {
        stack.pushVal(it->addr);
    }

    // argv
    stack.pushVal(0);
    for (auto it = args.rbegin(); it != args.rend(); it++) {
        stack.pushVal(it->addr);
    }
    stack.pushVal(args.size());
}

TaskControlBlock* createElfTask(uint8_t* file, size_t size, const char* const* argv, const char* const* envp) {
    auto tcb = new TaskControlBlock;

    auto [pageDir, brkAddr, entry] = loadElf(tcb->vmSpace, file, size);

    // stack
    tcb->vmSpace.insert(paging::VMA{
        0xBFFFF000_va,
        0xC0000000_va,
        MAP_PRIVATE | MAP_ANONYMOUS | MAP_GROWSDOWN,
        PROT_READ | PROT_WRITE,
        "stack",
    });

    Stack stack(pageDir, 0xC0000000_va);
    loadArgv(stack, argv, envp);
    stack.pushVal(stack.userEsp().addr);
    stack.pushVal(entry.addr);

    Stack kernelStack;
    fillStack(kernelStack, elfEntry, stack.userEsp().as<void>());

    tcb->userEsp = kernelStack.esp().addr;
    tcb->cr3 = pageDir.__mapper.paddr;
    tcb->kernelEsp = kernelStack.userBase.nextPage().addr;
    tcb->state = State::S_Ready;
    tcb->pid = KP_Invalid;

    tcb->parentPid = __scheduler->__current->pid;
    tcb->groupPid = __scheduler->__current->groupPid;

    tcb->name = lib::format("elf_{}", argv[0] ? argv[0] : "unknown");
    tcb->brkBase = brkAddr;
    tcb->brkAddr = brkAddr;
    tcb->stackRange = {0xC0000000_va - 0x800000, 0xC0000000_va};
    tcb->pages.push_back(kernelStack.userBase.addr);

    tcb->cwd = fs::rootEntry()->__mount_point;

    __scheduler->__current->childTasks.push_back(tcb);

    return tcb;
}

void execTask(uint8_t* file,
              size_t size,
              const char* const* argv,
              const char* const* envp,
              interrupt::SyscallFrame* frame) {
    paging::VMSpace vmSpace;

    auto [pageDir, brkAddr, entry] = loadElf(vmSpace, file, size);

    // stack
    vmSpace.insert(paging::VMA{
        0xBFFFF000_va,
        0xC0000000_va,
        MAP_PRIVATE | MAP_ANONYMOUS | MAP_GROWSDOWN,
        PROT_READ | PROT_WRITE,
        "stack",
    });

    auto tcb = __scheduler->__current;

    Stack stack(pageDir, 0xC0000000_va);
    loadArgv(stack, argv, envp);
    std::string name = lib::format("elf_{}", argv[0] ? argv[0] : "unknown");
    argv = nullptr;

    if (auto cr3 = tcb->cr3; cr3 != paging::kernelPageDirectory.cr3()) {
        auto userPage = paging::UserDirectory::from(cr3);
        tcb->vmSpace.free(userPage);
        userPage.freePageTables();
        allocator::physicalFrameRelease(cr3);
    } else {
        arch::kprint("perform exec on system task!\n");
    }

    tcb->userEsp = stack.userEsp().addr;
    tcb->cr3 = pageDir.__mapper.paddr;
    // tcb->kernelEsp
    // tcb->state
    // tcb->pid

    tcb->vmSpace = std::move(vmSpace);

    // tcb->parentPid
    // tcb->groupPid

    tcb->__signal.prepareForExec();

    // fdTable close-on-exec
    // tty
    tcb->tls = {};

    tcb->name = name;
    tcb->brkBase = brkAddr;
    tcb->brkAddr = brkAddr;
    tcb->stackRange = {0xC0000000_va - 0x800000, 0xC0000000_va};
    // tcb->pages

    // tcb->cwd

    pageDir.__mapper.paddr.setCr3();
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
    auto currPageDir = paging::UserDirectory::from(__scheduler->__current->cr3);
    auto newPageDir = paging::UserDirectory::forkCOW(currPageDir);
    __scheduler->__current->cr3.setCr3();

    Stack kernelStack;
    auto newFrame = kernelStack.pushAny(*frame);
    newFrame->eax = 0;
    fillStack(kernelStack, forkEntry, kernelStack.esp().as<void>());

    auto tcb = new TaskControlBlock;
    tcb->userEsp = kernelStack.esp().addr;
    tcb->cr3 = newPageDir.__mapper.paddr;
    tcb->kernelEsp = kernelStack.userBase.nextPage().addr;
    tcb->state = State::S_Ready;
    tcb->pid = KP_Invalid;

    tcb->vmSpace = __scheduler->__current->vmSpace;

    tcb->parentPid = __scheduler->__current->pid;
    tcb->groupPid = __scheduler->__current->groupPid;

    tcb->__signal.prepareForFork(__scheduler->__current->__signal);
    tcb->__file = __scheduler->__current->__file;

    tcb->tls = __scheduler->__current->tls;

    tcb->name = __scheduler->__current->name;
    tcb->brkBase = __scheduler->__current->brkBase;
    tcb->brkAddr = __scheduler->__current->brkAddr;
    tcb->stackRange = __scheduler->__current->stackRange;
    tcb->pages.push_back(kernelStack.userBase.addr);

    tcb->cwd = __scheduler->__current->cwd;

    __scheduler->__current->childTasks.push_back(tcb);

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

    if (__scheduler->__current->pid == KP_Init) {
        arch::kfatal("init task cannot exit!");
    }

    __scheduler->__current->state = State::S_Exited;
    __scheduler->__current->exitInfo.stat = (code << 8) | sig;
    if (!__scheduler->__current->childTasks.empty()) {
        __all_tasks[KP_Init]->childTasks.splice(__all_tasks[KP_Init]->childTasks.end(),
                                                __scheduler->__current->childTasks);
        __all_tasks[KP_Init]->sendSignal(SIGCHLD);
    }
    if (auto parent = findTask(__scheduler->__current->parentPid)) {
        parent->sendSignal(SIGCHLD);
    }
    if (!pendingTasks.empty()) {
        auto task = pendingTasks.front();
        pendingTasks.pop_front();
        switchToTask(task);
    } else {
        switchToTask(__all_tasks[KP_Idle]);
    }
    arch::kfatal("exited task rescheduled!");
}

bool freeTask(pid_t pid, int* stat) {
    arch::kprint("free task pid = {} current = {} {}\n", pid, __scheduler->__current->pid,
                 __scheduler->__current->name);
    auto task = __all_tasks[pid];
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
        task->vmSpace.free(userPage);
        userPage.freePageTables();
        allocator::physicalFrameRelease(task->cr3);
    }

    delete task;
    __all_tasks[pid] = nullptr;

    return true;
}

void yield() {
    arch::InterruptGuard guard;
    if (!pendingTasks.empty()) {
        auto next = pendingTasks.front();
        pendingTasks.pop_front();
        if (__scheduler->__current->state == State::S_Running) {
            __scheduler->__current->state = State::S_Ready;
            if (__scheduler->__current->pid != KP_Idle) {
                pendingTasks.push_back(__scheduler->__current);
            }
        }
        switchToTask(next);
        __scheduler->__current->state = State::S_Running;
        gdt::setTls(__scheduler->__current->tls);
    } else if (__scheduler->__current->state != State::S_Running) {
        switchToTask(__all_tasks[KP_Idle]);
        __scheduler->__current->state = State::S_Running;
        gdt::setTls(__scheduler->__current->tls);
    }
}

WakeReason block(BlockReason reason) {
    arch::InterruptGuard guard;
    __scheduler->__current->state = State::S_Blocked;
    __scheduler->__current->blockReason = reason;
    __scheduler->__current->wakeReason = WakeReason::WR_Normal;
    yield();
    auto wr = __scheduler->__current->wakeReason;
    __scheduler->__current->wakeReason = WakeReason::WR_Normal;
    return wr;
}

void unblock(TaskControlBlock* task, WakeReason reason) {
    if (task->state != State::S_Blocked) {
        return;
    }
    task->state = State::S_Ready;
    task->blockReason = BlockReason::BR_Unknown;
    task->wakeReason = reason;
    if (auto func = std::move(task->__request_detach)) {
        task->__request_detach.reset();
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
    __scheduler->__current->sleepInfo.time = currTs;

    auto pos = std::find_if(sleepTasks.begin(), sleepTasks.end(),
                            [&](const auto& tcb) { return currTs <= tcb.sleepInfo.time; });
    sleepTasks.insert(pos, __scheduler->__current);
    __scheduler->__current->__request_detach = +[](TaskControlBlock* task) { sleepTasks.erase({task}); };
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

void checkSleep() {
    arch::InterruptGuard guard;
    while (!sleepTasks.empty()) {
        auto task = sleepTasks.front();
        if (task->sleepInfo.time < timer::msSinceBoot) {
            sleepTasks.pop_front();
            task->__request_detach.reset();
            unblock(task, WakeReason::WR_Normal);
        } else {
            break;
        }
    }
    if ((timer::msSinceBoot % 10 == 0) && __scheduler->__current) {
        yield();
    }
}

}  // namespace nyan::task
