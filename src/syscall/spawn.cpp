#include <nyan/syscall.h>

#include "../arch/io.hpp"
#include "../data/embed.hpp"
#include "../task/task.hpp"
#include "../task/tcb.hpp"

namespace nyan::syscall {

pid_t spawn(const char* name, const char* const* argv) {
    for (auto prog : data::programs) {
        if (std::string_view{prog.name} == name) {
            auto task = task::createElfTask(prog.data, prog.size, argv);
            task->tty = task::currentTask->tty;
            auto pid = task::addTask(task);
            arch::kprint("spawn {} as {}\n", name, pid);
            return pid;
        }
    }
    return -1;
}

}  // namespace nyan::syscall
