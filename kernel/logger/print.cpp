#include "print.hpp"

#include "../task/scheduler.hpp"
#include "../task/tcb.hpp"
#include "../timer/load.hpp"

namespace nyan::logger {

void emitLog(void* eip, LogLevel level, std::string_view log) {
    Payload payload = {
        static_cast<uint32_t>(timer::msSinceBoot),
        reinterpret_cast<uint32_t>(eip),
        task::__scheduler->__current ? task::__scheduler->__current->pid : 0,
        static_cast<uint16_t>(log.length()),
        T_Log,
        {
            .logLevel = level,
        },
    };

    arch::InterruptGuard guard;
    arch::kwrite(&payload, sizeof(Payload));
    arch::kwrite(log.data(), static_cast<uint16_t>(log.length()));
}

void emitSyscall(uint32_t eip, SyscallRole role, const SyscallContent& content) {
    Payload payload = {
        static_cast<uint32_t>(timer::msSinceBoot),
        eip,
        task::__scheduler->__current ? task::__scheduler->__current->pid : 0,
        sizeof(SyscallContent),
        T_Syscall,
        {
            .syscallRole = role,
        },
    };

    arch::InterruptGuard guard;
    arch::kwrite(&payload, sizeof(Payload));
    arch::kwrite(&content, sizeof(SyscallContent));
}

void emitException(void* eip, const ExceptionContent& content) {
    Payload payload = {
        static_cast<uint32_t>(timer::msSinceBoot),
        reinterpret_cast<uint32_t>(eip),
        task::__scheduler->__current ? task::__scheduler->__current->pid : 0,
        sizeof(ExceptionContent),
        T_Exception,
        {},
    };

    arch::InterruptGuard guard;
    arch::kwrite(&payload, sizeof(Payload));
    arch::kwrite(&content, sizeof(ExceptionContent));
}

}  // namespace nyan::logger
