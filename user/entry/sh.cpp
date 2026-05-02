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

std::string cwd() {
    char buf[256];
    getcwd(buf, 256);
    return buf;
}

void printPrompt() {
    std::cout << cwd() << "$ " << std::flush;
}

int main() {
    printPrompt();
    std::string line;
    while (std::getline(std::cin, line)) {
        auto args = parseArgs(line);
        if (args.empty()) {
            printPrompt();
            continue;
        }

        if (args[0] == "exit") {
            break;
        } else if (args[0] == "cd") {
            if (args.size() > 1) {
                chdir(args[1].c_str());
            } else {
                chdir("/");
            }
            printPrompt();
            continue;
        } else if (args[0] == "pwd") {
            std::cout << cwd() << std::endl;
            printPrompt();
            continue;
        } else if (args[0] == "env") {
            for (auto ptr = environ; *ptr; ptr++) {
                std::cout << *ptr << std::endl;
            }
            printPrompt();
            continue;
        } else if (args[0] == "exec") {
            if (args.size() > 1) {
                auto execArgs = std::vector<std::string>(args.begin() + 1, args.end());
                auto argv = toCArgv(execArgs);
                if (execve(argv[0], argv.data(), environ) < 0) {
                    std::cout << "launch failed" << std::endl;
                    printPrompt();
                    continue;
                }
            } else {
                printPrompt();
                continue;
            }
        }

        auto argv = toCArgv(args);
        if (auto pid = fork()) {
            tcsetpgrp(0, pid);
            int stat;
            if (pid == waitpid(pid, &stat, 0)) {
                if (WIFEXITED(stat)) {
                    std::cout << "exit with " << WEXITSTATUS(stat) << std::endl;
                } else {
                    std::cout << "signal with " << WTERMSIG(stat) << std::endl;
                }
            } else {
                std::cout << "wait failed" << std::endl;
            }
            tcsetpgrp(0, getpid());
        } else {
            execve(argv[0], argv.data(), nullptr);
            std::cout << "launch failed" << std::endl;
        }

        printPrompt();
    }
    return 0;
}
