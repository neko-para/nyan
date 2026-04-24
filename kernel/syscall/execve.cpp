#include <nyan/syscall.h>

#include <string_view>

#include "../arch/print.hpp"
#include "../data/embed.hpp"
#include "../task/task.hpp"

namespace nyan::syscall {

int execve(const char* pathname,
           char* const argv[],
           [[maybe_unused]] char* const envp[],
           interrupt::SyscallFrame* frame) {
    for (size_t i = 0; i < data::programCount; i++) {
        const auto& prog = data::programs[i];
        if (std::string_view{prog.name} == pathname) {
            arch::kprint("exec {}\n", pathname);
            task::execTask(prog.data, prog.size, argv, frame);
            return 0;
        }
    }
    return -SYS_EACCES;
}

}  // namespace nyan::syscall
