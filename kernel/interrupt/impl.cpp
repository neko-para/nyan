#include "isr.hpp"

#include <nyan/syscall.h>

#include "../arch/instr.hpp"
#include "../arch/print.hpp"
#include "../gdt/load.hpp"
#include "../keyboard/mod.hpp"
#include "../lib/format.hpp"
#include "../paging/directory.hpp"
#include "../task/scheduler.hpp"
#include "../task/tcb.hpp"
#include "../timer/load.hpp"
#include "load.hpp"

namespace nyan::interrupt {

void exceptionHandlerImpl(SyscallFrame* frame) {
    if (frame->isr_num <= E_LastException) {
        logger::ExceptionContent content = {
            frame->isr_num, frame->error_code, frame->cs, frame->eip, arch::cr2(),
        };
        logger::emitException(0, content);
    }

    switch (frame->isr_num) {
        case E_Breakpoint:
            task::__scheduler->__current->sendSignal(SIGTRAP);
            break;

        case E_GeneralProtectionFault:
            if (gdt::isRing3(frame->cs)) {
                task::__scheduler->__current->sendSignal(SIGSEGV);
                break;
            }

            arch::kfatal();
            break;

        case E_PageFault: {
            paging::VirtualAddress targetAddr = paging::VirtualAddress{arch::cr2()};
            auto vma = task::__scheduler->__current->vmSpace.__locate(targetAddr);

            bool checkUserland = targetAddr.addr >= 0x00400000 && targetAddr.addr < 0xC0000000;

            constexpr auto COWFlag = PF_Present | PF_Write;
            bool checkCOW = checkUserland                                  //
                            && vma->contains(targetAddr)                   //
                            && ((frame->error_code & COWFlag) == COWFlag)  //
                            && task::__scheduler->__current->cr3 != paging::kernelPageDirectory.cr3();

            if (checkCOW) {
                auto pageDir = paging::UserDirectory::from(task::__scheduler->__current->cr3);
                if (pageDir.handleCOW(targetAddr)) {
                    return;
                }
            }

            if (gdt::isRing3(frame->cs)) {
                bool checkStack = checkUserland                                //
                                  && vma->bounds(*std::next(vma), targetAddr)  //
                                  && (std::next(vma)->__flags & MAP_GROWSDOWN);

                if (checkStack) {
                    // TODO: 之后可以引入基于ESP的检查
                    if (targetAddr >= task::__scheduler->__current->stackRange.first) {
                        auto pageDir = paging::UserDirectory::from(task::__scheduler->__current->cr3);
                        auto nextAddr = std::next(vma)->__begin;
                        while (nextAddr > targetAddr) {
                            nextAddr = nextAddr.prevPage();
                            pageDir.alloc(nextAddr, true);
                        }
                        break;
                    }
                }

                task::__scheduler->__current->sendSignal(SIGSEGV);
                break;
            }

            arch::kfatal();
            break;
        }

        case I_Timer:
            end(0);
            timer::hit();
            break;

        case I_Keyboard: {
            end(1);
            uint8_t ch = arch::inb(0x60);
            keyboard::push(ch);
            break;
        }

        case I_Syscall:
            syscallHandlerImpl(frame);
            break;

        default:
            arch::kfatal();
    }

    if (gdt::isRing3(frame->cs)) {
        task::__scheduler->__current->checkSignal(frame);
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

template <typename Ret, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5>
static void call(SyscallFrame* frame, Ret (*func)(Arg1, Arg2, Arg3, Arg4, Arg5)) {
    auto a1 = castArg<Arg1>(frame->ebx);
    auto a2 = castArg<Arg2>(frame->ecx);
    auto a3 = castArg<Arg3>(frame->edx);
    auto a4 = castArg<Arg4>(frame->esi);
    auto a5 = castArg<Arg5>(frame->edi);
    if constexpr (std::same_as<Ret, void>) {
        frame->eax = 0;
        func(a1, a2, a3, a4, a5);
    } else {
        frame->eax = castRet(func(a1, a2, a3, a4, a5));
    }
}

template <typename Ret, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6>
static void call(SyscallFrame* frame, Ret (*func)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6)) {
    auto a1 = castArg<Arg1>(frame->ebx);
    auto a2 = castArg<Arg2>(frame->ecx);
    auto a3 = castArg<Arg3>(frame->edx);
    auto a4 = castArg<Arg4>(frame->esi);
    auto a5 = castArg<Arg5>(frame->edi);
    auto a6 = castArg<Arg6>(frame->ebp);
    if constexpr (std::same_as<Ret, void>) {
        frame->eax = 0;
        func(a1, a2, a3, a4, a5, a6);
    } else {
        frame->eax = castRet(func(a1, a2, a3, a4, a5, a6));
    }
}

#define CALL(func) call(frame, syscall::func);

extern "C" void syscallHandlerImpl(SyscallFrame* frame) {
    // 简单处理, syscall中不可重入中断
    arch::InterruptGuard guard;

    logger::SyscallContent content = {
        0, frame->eax, frame->ebx, frame->ecx, frame->edx, frame->esi, frame->edi, frame->ebp,
    };
    logger::emitSyscall(frame->eip, logger::SR_Enter, content);

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
        case 12:
            CALL(chdir);
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
        // case 90:
        //     CALL(mmap);
        //     break;
        case 91:
            CALL(munmap);
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
        case 174:
            CALL(rt_sigaction);
            break;
        case 175:
            CALL(rt_sigprocmask);
            break;
        case 183:
            CALL(getcwd);
            break;
        case 192:
            CALL(mmap2);
            break;
        case 224:
            CALL(gettid);
            break;
        case 238:
            CALL(tkill);
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
            arch::kprint("syscall eax={}(missing) from {} {}\n", frame->eax, task::__scheduler->__current->pid,
                         task::__scheduler->__current->name);
            frame->eax = -SYS_ENOSYS;
    }

    content.ret = frame->eax;
    logger::emitSyscall(frame->eip, logger::SR_Leave, content);
}

}  // namespace nyan::interrupt
