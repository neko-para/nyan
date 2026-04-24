#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "utils.hpp"

bool readline(char* buf, size_t len) {
    if (fgets(buf, len, stdin)) {
        buf[strlen(buf) - 1] = 0;
        return true;
    } else {
        return false;
    }
}

void processArgv(char** argv, char* buf) {
    while (*buf) {
        if (*buf == ' ') {
            buf++;
            continue;
        }
        *argv++ = buf;
        char* next = strchr(buf, ' ');
        if (!next) {
            break;
        }
        *next++ = 0;
        buf = next;
    }
    *argv = 0;
}

extern "C" int main() {
    char buf[256];
    fputs("> ", stdout);
    fflush(stdout);
    while (readline(buf, 255)) {
        if (!strcmp(buf, "exit")) {
            break;
        } else {
            char* argv[128];
            processArgv(argv, buf);
            if (!argv[0]) {
                fputs("\n> ", stdout);
                fflush(stdout);
                continue;
            }
            if (!strcmp(argv[0], "exec")) {
                if (argv[1]) {
                    if (execve(argv[1], argv + 1, 0) < 0) {
                        fputs("launch failed\n\n> ", stdout);
                        fflush(stdout);
                        continue;
                    }
                } else {
                    fputs("\n> ", stdout);
                    fflush(stdout);
                    continue;
                }
            }

            auto pid = spawn(argv[0], argv);
            if (pid <= 0) {
                fputs("launch failed\n", stdout);
            } else {
                tcsetpgrp(0, pid);
                int stat;
                if (pid == waitpid(pid, &stat, 0)) {
                    if (WIFEXITED(stat)) {
                        fputs("exit with ", stdout);
                        printNum(WEXITSTATUS(stat), stdout);
                        fputc('\n', stdout);
                    } else {
                        fputs("signal with ", stdout);
                        printNum(WTERMSIG(stat), stdout);
                        fputc('\n', stdout);
                    }
                } else {
                    fputs("wait failed\n", stdout);
                }
                tcsetpgrp(0, getpid());
            }
        }
        fputs("\n> ", stdout);
        fflush(stdout);
    }
    return 0;
}
