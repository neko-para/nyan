#pragma once

#include <string_view>

#include "types.hpp"

namespace nyan::logger {

void emitLog(void* eip, LogLevel level, std::string_view log);
void emitSyscall(uint32_t eip, SyscallRole role, const SyscallContent& content);
void emitException(void* eip, const ExceptionContent& content);

}  // namespace nyan::logger
