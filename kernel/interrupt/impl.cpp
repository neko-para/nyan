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

template <uint32_t Id>
void defaultHandlerImpl(Frame* frame, uint32_t error) {
    if constexpr (Id == E_GeneralProtectionFault) {
        arch::kfatal("General Protection Fault: selector {#4x}", error);
    } else if constexpr (Id == E_PageFault) {
        paging::VirtualAddress targetAddr = paging::VirtualAddress{arch::cr2()};
        if (targetAddr.addr < 0xC0000000 &&
            (error & (PF_Present | PF_User | PF_Write)) == (PF_Present | PF_User | PF_Write) &&
            task::currentTask->cr3 != paging::kernelPageDirectory.cr3()) {
            auto pageDir = paging::UserDirectory::from(task::currentTask->cr3);
            if (pageDir.handleCOW(targetAddr)) {
                return;
            }
        }

        arch::kprint("Page Fault: {#010x} {}\n", arch::cr2(), error);
        arch::kprint("  pid {}\n", task::currentTask->pid);
        arch::kprint("  eip {#010x}\n", frame->eip);
        if (error & PF_Present) {
            arch::kputs("Present ");
        }
        if (error & PF_Write) {
            arch::kputs("Write ");
        }
        if (error & PF_User) {
            arch::kputs("User ");
        }
        arch::kfatal();
    } else {
        arch::kfatal("Exception {}: code {}", Id, error);
    }
}

template <uint32_t Id>
void defaultHandlerImplNe(Frame* frame) {
    if constexpr (Id == E_Breakpoint) {
        // TODO: SIGTRAP for Id 3
        arch::kputs("Breakpoint hit\n");
        arch::cli();
        arch::hlt();
        arch::sti();
    } else {
        arch::kfatal("Exception {}\n  pid {}\n  eip {}\n", Id, task::currentTask->pid, frame->eip);
    }
}

template void defaultHandlerImplNe<0>(Frame*);
template void defaultHandlerImplNe<1>(Frame*);
template void defaultHandlerImplNe<2>(Frame*);
template void defaultHandlerImplNe<3>(Frame*);
template void defaultHandlerImplNe<4>(Frame*);
template void defaultHandlerImplNe<5>(Frame*);
template void defaultHandlerImplNe<6>(Frame*);
template void defaultHandlerImplNe<7>(Frame*);
template void defaultHandlerImpl<8>(Frame*, uint32_t);
template void defaultHandlerImplNe<9>(Frame*);
template void defaultHandlerImpl<10>(Frame*, uint32_t);
template void defaultHandlerImpl<11>(Frame*, uint32_t);
template void defaultHandlerImpl<12>(Frame*, uint32_t);
template void defaultHandlerImpl<13>(Frame*, uint32_t);
template void defaultHandlerImpl<14>(Frame*, uint32_t);
template void defaultHandlerImplNe<15>(Frame*);
template void defaultHandlerImplNe<16>(Frame*);
template void defaultHandlerImpl<17>(Frame*, uint32_t);
template void defaultHandlerImplNe<18>(Frame*);
template void defaultHandlerImplNe<19>(Frame*);
template void defaultHandlerImplNe<20>(Frame*);
template void defaultHandlerImpl<21>(Frame*, uint32_t);
template void defaultHandlerImplNe<22>(Frame*);
template void defaultHandlerImplNe<23>(Frame*);
template void defaultHandlerImplNe<24>(Frame*);
template void defaultHandlerImplNe<25>(Frame*);
template void defaultHandlerImplNe<26>(Frame*);
template void defaultHandlerImplNe<27>(Frame*);
template void defaultHandlerImplNe<28>(Frame*);
template void defaultHandlerImpl<29>(Frame*, uint32_t);
template void defaultHandlerImpl<30>(Frame*, uint32_t);
template void defaultHandlerImplNe<31>(Frame*);

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
        case 243:
            CALL(set_thread_area);
            break;
        case 258:
            CALL(set_tid_address);
            break;
        case 512:
            CALL(spawn);
            break;
        default:
            frame->eax = -SYS_ENOSYS;
    }

    task::checkSignal(frame);
}

extern "C" void timerHandlerImpl(SyscallFrame* frame) {
    end(0);
    timer::hit(frame);
}

extern "C" void keyboardHandlerImpl(SyscallFrame* frame) {
    end(1);
    uint8_t ch = arch::inb(0x60);
    keyboard::push(ch, frame);

    if (gdt::isRing3(frame->cs)) {
        task::checkSignal(frame);
    }
}

}  // namespace nyan::interrupt
