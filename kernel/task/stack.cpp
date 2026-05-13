#include "stack.hpp"

#include <elf.h>

namespace nyan::task {

void loadArgv(Stack& stack, const std::vector<std::string>& argv, const std::vector<std::string>& envp) noexcept {
    std::vector<paging::VirtualAddress> args;
    for (const auto& arg : argv) {
        args.push_back(stack.translator.toUser(stack.pushString(arg)));
    }
    std::vector<paging::VirtualAddress> envs;
    for (const auto& env : envp) {
        envs.push_back(stack.translator.toUser(stack.pushString(env)));
    }

    // auxv
    stack.pushVal(0);
    stack.pushVal(AT_NULL);
    stack.pushVal(4096);
    stack.pushVal(AT_PAGESZ);

    // envp
    stack.pushVal(0);
    for (auto it = envs.rbegin(); it != envs.rend(); it++) {
        stack.pushVal(it->addr);
    }

    // argv
    stack.pushVal(0);
    for (auto it = args.rbegin(); it != args.rend(); it++) {
        stack.pushVal(it->addr);
    }
    stack.pushVal(args.size());
}

}  // namespace nyan::task
