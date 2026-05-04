#include "task.hpp"

#include <elf.h>
#include <vector>

#include "../allocator/mod.hpp"
#include "../arch/guard.hpp"
#include "../elf/entry.hpp"
#include "../elf/mod.hpp"
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

    __scheduler->exit(code);
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

TaskControlBlock* createElfTask(std::span<uint8_t> file,
                                std::vector<std::string> argv,
                                std::vector<std::string> env) noexcept {
    auto tcb = new TaskControlBlock;

    auto [pageDir, brkAddr, entry] = elf::loadElf(tcb->vmSpace, file);

    // stack
    tcb->vmSpace.insert(paging::VMA{
        0xBFFFF000_va,
        0xC0000000_va,
        MAP_PRIVATE | MAP_ANONYMOUS | MAP_GROWSDOWN,
        PROT_READ | PROT_WRITE,
        "stack",
    });

    Stack stack(pageDir, 0xC0000000_va);
    loadArgv(stack, argv, env);
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

    tcb->name = argv.size() > 0 ? argv[0] : "unknown";
    tcb->brkBase = brkAddr;
    tcb->brkAddr = brkAddr;
    tcb->stackRange = {0xC0000000_va - 0x800000, 0xC0000000_va};
    tcb->pages.push_back(kernelStack.userBase.addr);

    tcb->cwd = fs::rootEntry()->__mount_point;

    __scheduler->__current->childTasks.push_back(tcb);

    return tcb;
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

    auto pid = __scheduler->addTask(tcb);
    if (pid == KP_Invalid) {
        arch::kfatal("fork failed");
    }
    return pid;
}

}  // namespace nyan::task
