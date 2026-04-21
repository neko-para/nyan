#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "utils.hpp"

bool readline(char* buf, size_t len) {
    char* ptr = buf;
    while (true) {
        auto count = read(0, ptr, len - (ptr - buf));
        if (count <= 0) {
            *ptr = 0;
            return ptr != buf;
        }
        for (ssize_t i = 0; i < count; i++) {
            if (ptr[i] == '\n') {
                ptr[i] = 0;
                return true;
            }
        }
        ptr += count;
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
    // TODO: 之后换成fgets/scanf的话, 本身会自动flush
    fflush(stdout);
    while (readline(buf, 255)) {
        if (!strcmp(buf, "exit")) {
            break;
        } else {
            char* argv[128];
            processArgv(argv, buf);
            if (!argv[0]) {
                fputs("\n> ", stdout);
                continue;
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
