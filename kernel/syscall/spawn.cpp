#include <nyan/syscall.h>

#include "../arch/print.hpp"
#include "../data/embed.hpp"
#include "../task/scheduler.hpp"
#include "../task/task.hpp"
#include "../task/tcb.hpp"

namespace nyan::syscall {

pid_t spawn(const char* name, const char* const* argv, const char* const* envp) {
    for (size_t i = 0; i < data::programCount; i++) {
        const auto& prog = data::programs[i];
        if (std::string_view{prog.name} == name) {
            std::vector<std::string> args;
            std::vector<std::string> envs;

            for (auto ptr = argv; *ptr; ptr++) {
                args.push_back(*ptr);
            }

            for (auto ptr = envp; *ptr; ptr++) {
                envs.push_back(*ptr);
            }

            auto task = task::createElfTask(std::span{prog.data, prog.size}, args, envs);
            // TODO: close-on-exec
            task->__file = task::__scheduler->__current->__file;
            auto pid = task::__scheduler->addTask(task);
            arch::kprint("spawn {} as {}\n", name, pid);
            return pid;
        }
    }
    return -SYS_ENOENT;
}

}  // namespace nyan::syscall
