#include "scheduler.hpp"

#include <elf.h>

#include "../arch/guard.hpp"
#include "../elf/mod.hpp"
#include "../gdt/load.hpp"
#include "../paging/vma.hpp"
#include "pid.hpp"
#include "stack.hpp"
#include "tcb.hpp"

namespace nyan::task {

pid_t Scheduler::addTask(TaskControlBlock* task) noexcept {
    arch::InterruptGuard guard;
    __pending.push_back(task);
    if (task->pid == KP_Invalid) {
        return allocPid(task);
    } else {
        return task->pid;
    }
}

bool Scheduler::freeTask(pid_t pid, int* stat) noexcept {
    arch::kprint("free task pid = {} current = {} {}\n", pid, __current->pid, __current->name);
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

void Scheduler::execTask(std::span<uint8_t> file,
                         std::vector<std::string> argv,
                         std::vector<std::string> env,
                         interrupt::SyscallFrame* frame) noexcept {
    paging::VMSpace vmSpace;

    auto [pageDir, brkAddr, entry] = elf::loadElf(vmSpace, file);

    // stack
    vmSpace.insert(paging::VMA{
        0xBFFFF000_va,
        0xC0000000_va,
        MAP_PRIVATE | MAP_ANONYMOUS | MAP_GROWSDOWN,
        PROT_READ | PROT_WRITE,
        "stack",
    });

    Stack stack(pageDir, 0xC0000000_va);
    loadArgv(stack, argv, env);
    std::string name = argv.size() > 0 ? argv[0] : "unknown";

    if (auto cr3 = __current->cr3; cr3 != paging::kernelPageDirectory.cr3()) {
        auto userPage = paging::UserDirectory::from(cr3);
        __current->vmSpace.free(userPage);
        userPage.freePageTables();
        allocator::physicalFrameRelease(cr3);
    } else {
        arch::kprint("perform exec on system task!\n");
    }

    __current->userEsp = stack.userEsp().addr;
    __current->cr3 = pageDir.__mapper.paddr;
    // __current->kernelEsp
    // __current->state
    // __current->pid

    __current->vmSpace = std::move(vmSpace);

    // __current->parentPid
    // __current->groupPid

    __current->__signal.prepareForExec();

    // fdTable close-on-exec
    // tty
    __current->tls = {};

    __current->name = name;
    __current->brkBase = brkAddr;
    __current->brkAddr = brkAddr;
    __current->stackRange = {0xC0000000_va - 0x800000, 0xC0000000_va};
    // __current->pages

    // __current->cwd
    __current->argv = std::move(argv);
    __current->env = std::move(env);

    pageDir.__mapper.paddr.setCr3();
    gdt::setTls(__current->tls);

    frame->eip = entry.addr;
    frame->user_esp = __current->userEsp;
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

}  // namespace nyan::task