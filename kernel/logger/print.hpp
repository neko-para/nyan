#pragma once

#include <string_view>

#include "types.hpp"

#define FROM_HERE __builtin_return_address()

namespace nyan::logger {

// __builtin_return_address

void emitSyscall(void* eip, SyscallRole role, const SyscallContent& content);
void emitLog(void* eip, LogLevel level, std::string_view log);

}  // namespace nyan::logger
