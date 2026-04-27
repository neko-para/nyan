#include "isr.hpp"

#include <nyan/syscall.h>

#include "../arch/instr.hpp"
#include "../arch/print.hpp"
#include "../gdt/load.hpp"
#include "../keyboard/load.hpp"
#include "../lib/format.hpp"
#include "../paging/directory.hpp"
#include "../task/task.hpp"
#include "../task/tcb.hpp"
#include "../timer/load.hpp"
#include "load.hpp"

namespace nyan::interrupt {

void exceptionHandlerImpl(SyscallFrame* frame) {
    switch (frame->isr_num) {
        case E_Breakpoint:
            task::sendSignal(task::currentTask, SIGTRAP);
            break;

        case E_GeneralProtectionFault:
            if (gdt::isRing3(frame->cs)) {
                task::sendSignal(task::currentTask, SIGSEGV);
                break;
            }

            arch::kfatal("General Protection Fault: selector {#4x}\n  pid {}\n  eip {#010x}\n", frame->error_code,
                         task::currentTask->pid, frame->eip);
            break;

        case E_PageFault: {
            paging::VirtualAddress targetAddr = paging::VirtualAddress{arch::cr2()};
            std::vector<paging::VMA>::iterator next;
            if (auto vma = task::currentTask->vmSpace.find(targetAddr, next);
                vma != task::currentTask->vmSpace.__addrs.end()) {
                if (targetAddr.addr < 0xC0000000 &&
                    (frame->error_code & (PF_Present | PF_User | PF_Write)) == (PF_Present | PF_User | PF_Write) &&
                    task::currentTask->cr3 != paging::kernelPageDirectory.cr3()) {
                    auto pageDir = paging::UserDirectory::from(task::currentTask->cr3);
                    if (pageDir.handleCOW(targetAddr)) {
                        return;
                    }
                }
            }

            if (gdt::isRing3(frame->cs)) {
                if (next != task::currentTask->vmSpace.__addrs.end() && next->__flags & MAP_GROWSDOWN &&
                    next->__end == task::currentTask->stackRange.second) {
                    // TODO: 之后可以引入基于ESP的检查
                    if (targetAddr >= task::currentTask->stackRange.first) {
                        auto pageDir = paging::UserDirectory::from(task::currentTask->cr3);
                        while (next->__begin > targetAddr) {
                            next->__begin = next->__begin.prevPage();
                            auto guard = pageDir.alloc(next->__begin, true);
                            memset(guard.as<void>(), 0, 4096);
                        }
                        break;
                    }
                }

                task::sendSignal(task::currentTask, SIGSEGV);
                break;
            }

            arch::kprint("Page Fault: {#010x} {}\n", arch::cr2(), frame->error_code);
            arch::kprint("  pid {}\n", task::currentTask->pid);
            arch::kprint("  eip {#010x}\n", frame->eip);
            if (frame->error_code & PF_Present) {
                arch::kputs("Present ");
            }
            if (frame->error_code & PF_Write) {
                arch::kputs("Write ");
            }
            if (frame->error_code & PF_User) {
                arch::kputs("User ");
            }
            arch::kfatal();
        }

        case I_Timer:
            end(0);
            timer::hit();
            break;

        case I_Keyboard: {
            end(1);
            uint8_t ch = arch::inb(0x60);
            keyboard::push(ch, frame);
            break;
        }

        case I_Syscall:
            syscallHandlerImpl(frame);
            break;

        default:
            arch::kfatal("Exception {}: code {}\n  pid {}\n  eip {#010x}\n", frame->isr_num, frame->error_code,
                         task::currentTask->pid, frame->eip);
    }

    if (gdt::isRing3(frame->cs)) {
        task::checkSignal(frame);
    }
}

template <typename T>
static T castArg(uint32_t val) {
    if constexpr (std::is_pointer_v<T>) {
        return reinterpret_cast<T>(val);
    } else {
        return static_cast<T>(val);
    }
}

template <typename T>
static uint32_t castRet(T val) {
    if constexpr (std::is_pointer_v<T>) {
        return reinterpret_cast<uint32_t>(val);
    } else {
        return static_cast<uint32_t>(val);
    }
}

template <typename Ret>
static void call(SyscallFrame* frame, Ret (*func)()) {
    if constexpr (std::same_as<Ret, void>) {
        frame->eax = 0;
        func();
    } else {
        frame->eax = castRet(func());
    }
}

template <typename Ret, typename Arg1>
static void call(SyscallFrame* frame, Ret (*func)(Arg1)) {
    auto a1 = castArg<Arg1>(frame->ebx);
    if constexpr (std::same_as<Ret, void>) {
        frame->eax = 0;
        func(a1);
    } else {
        frame->eax = castRet(func(a1));
    }
}

template <typename Ret, typename Arg1, typename Arg2>
static void call(SyscallFrame* frame, Ret (*func)(Arg1, Arg2)) {
    auto a1 = castArg<Arg1>(frame->ebx);
    auto a2 = castArg<Arg2>(frame->ecx);
    if constexpr (std::same_as<Ret, void>) {
        frame->eax = 0;
        func(a1, a2);
    } else {
        frame->eax = castRet(func(a1, a2));
    }
}

template <typename Ret, typename Arg1, typename Arg2, typename Arg3>
static void call(SyscallFrame* frame, Ret (*func)(Arg1, Arg2, Arg3)) {
    auto a1 = castArg<Arg1>(frame->ebx);
    auto a2 = castArg<Arg2>(frame->ecx);
    auto a3 = castArg<Arg3>(frame->edx);
    if constexpr (std::same_as<Ret, void>) {
        frame->eax = 0;
        func(a1, a2, a3);
    } else {
        frame->eax = castRet(func(a1, a2, a3));
    }
}

template <typename Ret, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
static void call(SyscallFrame* frame, Ret (*func)(Arg1, Arg2, Arg3, Arg4)) {
    auto a1 = castArg<Arg1>(frame->ebx);
    auto a2 = castArg<Arg2>(frame->ecx);
    auto a3 = castArg<Arg3>(frame->edx);
    auto a4 = castArg<Arg4>(frame->esi);
    if constexpr (std::same_as<Ret, void>) {
        frame->eax = 0;
        func(a1, a2, a3, a4);
    } else {
        frame->eax = castRet(func(a1, a2, a3, a4));
    }
}

template <size_t N>
static void dump(SyscallFrame* frame, const char (&name)[N]) {
    arch::kprint("syscall eax={}({}) from {} {}\n", frame->eax, name, task::currentTask->pid, task::currentTask->name);
}

#define CALL(func)      \
    dump(frame, #func); \
    call(frame, syscall::func);

extern "C" void syscallHandlerImpl(SyscallFrame* frame) {
    // 简单处理, syscall中不可重入中断
    arch::InterruptGuard guard;
    switch (frame->eax) {
        case 1:
            CALL(exit)
            break;
        case 2:
            frame->eax = syscall::fork(frame);
            break;
        case 3:
            CALL(read)
            break;
        case 4:
            CALL(write)
            break;
        case 6:
            CALL(close);
            break;
        case 7:
            CALL(waitpid)
            break;
        case 11:
            frame->eax = syscall::execve(castArg<const char*>(frame->ebx), castArg<char* const*>(frame->ecx),
                                         castArg<char* const*>(frame->edx), frame);
            break;
        case 20:
            CALL(getpid)
            break;
        case 37:
            CALL(kill)
            break;
        case 41:
            CALL(dup);
            break;
        case 42:
            CALL(pipe);
            break;
        case 45:
            CALL(brk)
            break;
        case 48:
            CALL(signal)
            break;
        case 54:
            CALL(ioctl);
            break;
        case 63:
            CALL(dup2);
            break;
        case 114:
            CALL(wait4);
            break;
        case 119:
            syscall::sigreturn(frame);
            break;
        case 145:
            CALL(readv);
            break;
        case 146:
            CALL(writev);
            break;
        case 162:
            CALL(nanosleep)
            break;
        case 175:
            CALL(rt_sigprocmask);
            break;
        case 243:
            CALL(set_thread_area);
            break;
        case 252:
            CALL(exit_group);
            break;
        case 258:
            CALL(set_tid_address);
            break;
        case 512:
            CALL(spawn);
            break;
        default:
            arch::kprint("syscall eax={}(missing) from {} {}\n", frame->eax, task::currentTask->pid,
                         task::currentTask->name);
            frame->eax = -SYS_ENOSYS;
    }
}

}  // namespace nyan::interrupt
