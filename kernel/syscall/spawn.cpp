#include <nyan/syscall.h>

#include "../arch/print.hpp"
#include "../data/embed.hpp"
#include "../task/task.hpp"
#include "../task/tcb.hpp"

namespace nyan::syscall {

pid_t spawn(const char* name, const char* const* argv) {
    for (size_t i = 0; i < data::programCount; i++) {
        const auto& prog = data::programs[i];
        if (std::string_view{prog.name} == name) {
            auto task = task::createElfTask(prog.data, prog.size, argv);
            // TODO: close-on-exec
            task->fdTable = task::currentTask->fdTable;
            task->tty = task::currentTask->tty;
            auto pid = task::addTask(task);
            arch::kprint("spawn {} as {}\n", name, pid);
            return pid;
        }
    }
    return -1;
}

}  // namespace nyan::syscall
