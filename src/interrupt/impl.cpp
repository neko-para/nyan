#include "isr.hpp"

#include <nyan/syscall.h>

#include "../arch/io.hpp"
#include "../lib/format.hpp"
#include "../task/tcb.hpp"

namespace nyan::interrupt {

template <uint32_t Id>
void defaultHandlerImpl(Frame* frame, uint32_t error) {
    if constexpr (Id == E_GeneralProtectionFault) {
        arch::kfatal("General Protection Fault: selector {#4x}", error);
    } else if constexpr (Id == E_PageFault) {
        arch::kprint("Page Fault: {#010x} {}\n", arch::cr2(), error);
        arch::kprint("  pid={}\n", task::currentTask->pid);
        arch::kprint("  eip={#010x}\n", frame->eip);
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
void defaultHandlerImplNe(Frame*) {
    if constexpr (Id == E_Breakpoint) {
        // TODO: SIGTRAP for Id 3
        arch::kputs("Breakpoint hit\n");
        arch::cli();
        arch::hlt();
        arch::sti();
    } else {
        arch::kfatal("Exception {}", Id);
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
    switch (frame->eax) {
        case 1:
            CALL(exit)
            return;
        case 2:
            CALL(spawn)
            return;
        case 3:
            CALL(read)
            return;
        case 4:
            CALL(write)
            return;
        case 7:
            CALL(waitpid)
            return;
        case 20:
            CALL(getpid)
            return;
        case 45:
            CALL(brk)
            return;
        case 162:
            CALL(nanosleep)
            return;
    }
    frame->eax = -SYS_ENOSYS;
}

}  // namespace nyan::interrupt
