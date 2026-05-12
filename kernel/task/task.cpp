#include "task.hpp"

#include <elf.h>
#include <vector>

#include "../fs/mod.hpp"
#include "../gdt/load.hpp"
#include "../interrupt/isr.hpp"
#include "../paging/directory.hpp"
#include "scheduler.hpp"
#include "stack.hpp"
#include "switch.hpp"
#include "tcb.hpp"

namespace nyan::task {

void taskWrapper(int (*func)(void* param), void* param) noexcept {
    __scheduler->__current->state = State::S_Running;
    arch::sti();

    auto code = func(param);

    __scheduler->exit(code);
}

void fillStack(Stack& stack, int (*func)(void* param), void* param) noexcept {
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

TaskControlBlock* createTask(int (*func)(void* param), void* param) noexcept {
    Stack stack;
    fillStack(stack, func, param);
    auto tcb = new TaskControlBlock;
    tcb->userEsp = stack.esp().addr;
    tcb->cr3 = paging::kernelPageDirectory.cr3();
    tcb->kernelEsp = stack.userBase.nextPage().addr;
    tcb->state = State::S_Ready;
    tcb->pid = KP_Invalid;

    tcb->parentPid = __scheduler->__current->pid;
    tcb->groupPid = __scheduler->__current->groupPid;
    tcb->brkBase = 0x400000_va;
    tcb->brkAddr = 0x400000_va;
    tcb->stackRange = {0xC0000000_va - 0x800000, 0xC0000000_va};
    tcb->pages.push_back(stack.userBase.addr);

    tcb->cwd = fs::rootEntry()->__mount_point;

    __scheduler->__current->childTasks.push_back(tcb);

    return tcb;
}

struct ExecEntryParam {
    std::span<uint8_t> file;
    std::vector<std::string> argv;
    std::vector<std::string> env;
};

static int execEntry(void* param) noexcept {
    arch::cli();
    auto info = reinterpret_cast<ExecEntryParam*>(param);
    interrupt::SyscallFrame frame{};
    __scheduler->execTask(info->file, std::move(info->argv), std::move(info->env), &frame);
    delete info;
    syscallReturn(&frame);
}

TaskControlBlock* createElfTask(std::span<uint8_t> file,
                                std::vector<std::string> argv,
                                std::vector<std::string> env) noexcept {
    auto param = new ExecEntryParam{
        file,
        std::move(argv),
        std::move(env),
    };
    auto tcb = createTask(execEntry, param);
    return tcb;
}

static int forkEntry(void* param) noexcept {
    arch::cli();
    syscallReturn(param);
    return 0;
}

pid_t forkTask(interrupt::SyscallFrame* frame) noexcept {
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
