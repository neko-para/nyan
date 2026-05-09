#include <nyan/syscall.h>

#include "../gdt/load.hpp"
#include "../task/mod.hpp"
#include "../task/scheduler.hpp"
#include "../task/signal.hpp"
#include "../task/tcb.hpp"

namespace nyan::syscall {

void sigreturn(interrupt::SyscallFrame* sysFrame) {
    if (!task::checkR(sysFrame)) {
        task::__scheduler->exit(255, SIGSEGV);
    }

    auto esp = sysFrame->user_esp;
    esp += 4;  // signum
    task::__scheduler->__current->__signal.__signal_mask = *reinterpret_cast<task::SigSet*>(esp);
    esp += sizeof(task::SigSet);  // sigmask
    auto userFrame = reinterpret_cast<interrupt::SyscallFrame*>(esp);
    *sysFrame = *userFrame;

    if (sysFrame->eip >= 0xC0000000 || !task::checkE(sysFrame->eip)) {
        task::__scheduler->exit(255, SIGSEGV);
    }

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
