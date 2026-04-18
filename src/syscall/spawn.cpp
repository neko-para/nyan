#include <nyan/syscall.h>

#include "../data/embed.hpp"
#include "../task/task.hpp"
#include "../task/tcb.hpp"

namespace nyan::syscall {

pid_t spawn(const char* name, const char* const* argv) {
    for (auto prog : data::programs) {
        if (std::string_view{prog.name} == name) {
            auto task = task::createElfTask(prog.data, prog.size, argv);
            task->tty = task::currentTask->tty;
            return task::addTask(task);
        }
    }
    return -1;
}

}  // namespace nyan::syscall
