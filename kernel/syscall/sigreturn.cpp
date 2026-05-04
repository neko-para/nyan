#include <nyan/syscall.h>

#include "../gdt/load.hpp"
#include "../task/scheduler.hpp"
#include "../task/signal.hpp"
#include "../task/tcb.hpp"

namespace nyan::syscall {

void sigreturn(void* frame) {
    interrupt::SyscallFrame* sysFrame = static_cast<interrupt::SyscallFrame*>(frame);

    auto esp = sysFrame->user_esp;
    esp += 4;  // signum
    task::__scheduler->__current->__signal.__signal_mask = *reinterpret_cast<task::SigSet*>(esp);
    esp += sizeof(task::SigSet);  // sigmask
    auto userFrame = reinterpret_cast<interrupt::SyscallFrame*>(esp);
    *sysFrame = *userFrame;

    sysFrame->cs = gdt::userCs;
    sysFrame->user_ss = gdt::userDs;
    sysFrame->user_ds = gdt::userDs;
    sysFrame->user_es = gdt::userDs;
    sysFrame->user_fs = gdt::userDs;
    if (sysFrame->user_gs != gdt::userDs && sysFrame->user_gs != gdt::userTls) {
        sysFrame->user_gs = gdt::userDs;
    }
    sysFrame->flags = (sysFrame->flags & ~0x3000) | 0x200;
}

}  // namespace nyan::syscall
