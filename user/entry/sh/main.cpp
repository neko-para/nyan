#include <array>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>

#include "parser.hpp"

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

std::optional<int> lastRet;

void printPrompt() {
    if (lastRet) {
        std::cout << *lastRet << ' ';
        lastRet = std::nullopt;
    }
    std::cout << cwd() << "$ " << std::flush;
}

bool isBuiltin(std::string_view name) {
    return name == "exit" || name == "cd" || name == "env" || name == "exec";
}

void doBuiltin(const std::vector<std::string>& args) {
    if (args[0] == "exit") {
        if (args.size() >= 2) {
            exit(std::stoi(args[1]));
        } else {
            exit(0);
        }
    } else if (args[0] == "cd") {
        if (args.size() > 1) {
            chdir(args[1].c_str());
        } else {
            chdir("/");
        }
    } else if (args[0] == "env") {
        for (auto ptr = environ; *ptr; ptr++) {
            std::cout << *ptr << std::endl;
        }
    } else if (args[0] == "exec") {
        if (args.size() > 1) {
            auto execArgs = std::vector<std::string>(args.begin() + 1, args.end());
            auto argv = toCArgv(execArgs);
            if (execvpe(argv[0], argv.data(), environ) < 0) {
                std::cout << "launch failed" << std::endl;
            }
        }
    }
}

int main() {
    setpgid(0, getpid());

    printPrompt();
    std::string line;
    while (std::getline(std::cin, line)) {
        auto cmds = parseCommand(line);

        if (cmds.empty()) {
            goto end;
        } else if (cmds.size() == 1 && isBuiltin(cmds[0].__argv[0])) {
            doBuiltin(cmds[0].__argv);
        } else {
            std::vector<std::array<int, 2>> pipes;
            std::vector<pid_t> pids;
            pipes.resize(cmds.size() - 1);
            for (size_t i = 0; i + 1 < cmds.size(); i++) {
                pipe(pipes[i].data());
            }

            pid_t pgid = -1;
            for (size_t i = 0; i < cmds.size(); i++) {
                auto& cmd = cmds[i];
                pid_t pid = fork();
                if (pid == 0) {
                    if (pgid == -1) {
                        setpgid(0, getpid());
                    } else {
                        setpgid(0, pgid);
                    }
                    if (i > 0) {
                        dup2(pipes[i - 1][0], 0);
                    }
                    if (i + 1 < cmds.size()) {
                        dup2(pipes[i][1], 1);
                    }
                    if (cmd.__stdin && !cmd.__stdin->empty()) {
                        int fd = open(cmd.__stdin->c_str(), O_RDONLY);
                        dup2(fd, 0);
                        close(fd);
                    }
                    if (cmd.__stdout && !cmd.__stdout->empty()) {
                        if (cmd.__stdout_append) {
                            puts("stdout append!");
                        }
                        int fd = open(cmd.__stdout->c_str(), O_WRONLY | O_CREAT | (cmd.__stdout_append ? O_APPEND : 0));
                        dup2(fd, 1);
                        close(fd);
                    }
                    if (cmd.__stderr && !cmd.__stderr->empty()) {
                        int fd = open(cmd.__stderr->c_str(), O_WRONLY | O_CREAT | (cmd.__stderr_append ? O_APPEND : 0));
                        dup2(fd, 2);
                        close(fd);
                    }
                    if (isBuiltin(cmd.__argv[0])) {
                        doBuiltin(cmd.__argv);
                    } else {
                        for (size_t j = 0; j + 1 < cmds.size(); j++) {
                            close(pipes[j][0]);
                            close(pipes[j][1]);
                        }

                        auto argv = toCArgv(cmd.__argv);
                        execvpe(argv[0], argv.data(), environ);
                        std::cout << "launch failed" << std::endl;
                    }
                } else {
                    if (i == 0) {
                        pgid = pid;
                    }
                    setpgid(pid, pgid);
                    if (i == 0) {
                        tcsetpgrp(0, pgid);
                    }
                    pids.push_back(pid);
                }
            }

            for (size_t i = 0; i + 1 < cmds.size(); i++) {
                close(pipes[i][0]);
                close(pipes[i][1]);
            }

            for (auto pid : pids) {
                int stat;
                if (pid == waitpid(pid, &stat, 0)) {
                    lastRet = WEXITSTATUS(stat);
                    if (!WIFEXITED(stat)) {
                        std::cout << "signal with " << WTERMSIG(stat) << std::endl;
                    }
                } else {
                    std::cout << "wait failed" << std::endl;
                }
            }

            tcsetpgrp(0, getpid());
        }

    end:
        printPrompt();
    }
    return 0;
}
