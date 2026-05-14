#include "isr.hpp"

#include <nyan/syscall.h>

#include "../arch/guard.hpp"
#include "../arch/instr.hpp"
#include "../arch/print.hpp"
#include "../gdt/load.hpp"
#include "../keyboard/mod.hpp"
#include "../lib/format.hpp"
#include "../paging/directory.hpp"
#include "../task/mod.hpp"
#include "../task/scheduler.hpp"
#include "../task/tcb.hpp"
#include "../timer/load.hpp"
#include "dispatch.hpp"
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
            task::__scheduler->raise(SIGTRAP);
            break;

        case E_GeneralProtectionFault:
            if (gdt::isRing3(frame->cs)) {
                volatile int gpf_wait = 1;
                while (gpf_wait) {
                    asm volatile("pause");
                }

                task::__scheduler->raise(SIGSEGV);
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
                if (task::checkStack(targetAddr)) {
                    break;
                }

                task::__scheduler->raise(SIGSEGV);
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
        task::__scheduler->checkSignal(frame);
    }
}

#define CALL(func) call(frame, syscall::func)

extern "C" void syscallHandlerImpl(SyscallFrame* frame) {
    if (frame->eax == 383) {  // statx
        frame->eax = SYS_ENOSYS;
        return;
    }

    // 简单处理, syscall中不可重入中断
    arch::InterruptGuard guard;

    logger::SyscallContent content = {
        0, frame->eax, frame->ebx, frame->ecx, frame->edx, frame->esi, frame->edi, frame->ebp,
    };
    logger::emitSyscall(frame->eip, logger::SR_Enter, content);

    switch (frame->eax) {
#pragma mark - BEGIN
        case 1:
            CALL(exit);
            break;
        case 2:
            CALL(fork);
            break;
        case 3:
            CALL(read);
            break;
        case 4:
            CALL(write);
            break;
        case 5:
            CALL(open);
            break;
        case 6:
            CALL(close);
            break;
        case 7:
            CALL(waitpid);
            break;
        case 11:
            CALL(execve);
            break;
        case 12:
            CALL(chdir);
            break;
        case 20:
            CALL(getpid);
            break;
        case 37:
            CALL(kill);
            break;
        case 41:
            CALL(dup);
            break;
        case 42:
            CALL(pipe);
            break;
        case 45:
            CALL(brk);
            break;
        case 54:
            CALL(ioctl);
            break;
        case 57:
            CALL(setpgid);
            break;
        case 63:
            CALL(dup2);
            break;
        case 64:
            CALL(getppid);
            break;
        case 65:
            CALL(getpgrp);
            break;
        case 85:
            CALL(readlink);
            break;
        case 91:
            CALL(munmap);
            break;
        case 114:
            CALL(wait4);
            break;
        case 119:
            CALL(sigreturn);
            break;
        case 132:
            CALL(getpgid);
            break;
        case 140:
            CALL(llseek);
            break;
        case 145:
            CALL(readv);
            break;
        case 146:
            CALL(writev);
            break;
        case 162:
            CALL(nanosleep);
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
            CALL(mmap_pgoff);
            break;
        case 195:
            CALL(stat64);
            break;
        case 196:
            CALL(lstat64);
            break;
        case 220:
            CALL(getdents64);
            break;
        case 221:
            CALL(fcntl64);
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
#pragma mark - END
        default:
            if (frame->eax == 407) {
                auto rqtp = reinterpret_cast<struct timespec*>(frame->edx);
                arch::kprint("407 sec {#010x} nsec {#010x}", rqtp->tv_sec, rqtp->tv_nsec);
            }

            arch::kprint("syscall eax={}(missing) from {} {}\n", frame->eax, task::__scheduler->__current->pid,
                         task::__scheduler->__current->name);
            frame->eax = SYS_ENOSYS;
    }

    content.ret = frame->eax;
    logger::emitSyscall(frame->eip, logger::SR_Leave, content);
}

}  // namespace nyan::interrupt
