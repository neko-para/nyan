#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <sys/wait.h>
#include <unistd.h>

std::vector<std::string> parseArgs(const std::string& line) {
    std::vector<std::string> args;
    std::istringstream iss(line);
    std::string token;
    while (iss >> token) {
        args.push_back(std::move(token));
    }
    return args;
}

std::vector<char*> toCArgv(std::vector<std::string>& args) {
    std::vector<char*> argv;
    for (auto& arg : args) {
        argv.push_back(arg.data());
    }
    argv.push_back(nullptr);
    return argv;
}

int main() {
    std::string line;
    std::cout << "> " << std::flush;
    while (std::getline(std::cin, line)) {
        if (line == "exit") {
            break;
        }

        auto args = parseArgs(line);
        if (args.empty()) {
            std::cout << "\n> " << std::flush;
            continue;
        }

        if (args[0] == "exec") {
            if (args.size() > 1) {
                auto execArgs = std::vector<std::string>(args.begin() + 1, args.end());
                auto argv = toCArgv(execArgs);
                if (execve(argv[0], argv.data(), nullptr) < 0) {
                    std::cout << "launch failed\n\n> " << std::flush;
                    continue;
                }
            } else {
                std::cout << "\n> " << std::flush;
                continue;
            }
        }

        auto argv = toCArgv(args);
        if (auto pid = fork()) {
            tcsetpgrp(0, pid);
            int stat;
            if (pid == waitpid(pid, &stat, 0)) {
                if (WIFEXITED(stat)) {
                    std::cout << "exit with " << WEXITSTATUS(stat) << "\n";
                } else {
                    std::cout << "signal with " << WTERMSIG(stat) << "\n";
                }
            } else {
                std::cout << "wait failed\n";
            }
            tcsetpgrp(0, getpid());
        } else {
            execve(argv[0], argv.data(), nullptr);
            std::cout << "launch failed\n";
        }

        std::cout << "\n> " << std::flush;
    }
    return 0;
}
