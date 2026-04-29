#include "print.hpp"

#include "../task/tcb.hpp"
#include "../timer/load.hpp"

namespace nyan::logger {

void emitLog(void* eip, LogLevel level, std::string_view log) {
    Payload payload = {
        static_cast<uint32_t>(timer::msSinceBoot),
        reinterpret_cast<uint32_t>(eip),
        task::currentTask ? task::currentTask->pid : 0,
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

void emitSyscall(void* eip, SyscallRole role, const SyscallContent& content) {
    Payload payload = {
        static_cast<uint32_t>(timer::msSinceBoot),
        reinterpret_cast<uint32_t>(eip),
        task::currentTask ? task::currentTask->pid : 0,
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
        task::currentTask ? task::currentTask->pid : 0,
        sizeof(ExceptionContent),
        T_Exception,
        {},
    };

    arch::InterruptGuard guard;
    arch::kwrite(&payload, sizeof(Payload));
    arch::kwrite(&content, sizeof(ExceptionContent));
}

}  // namespace nyan::logger
