#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

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

extern "C" int main() {
    char buf[256];
    fputs("> ", stdout);
    while (readline(buf, 255)) {
        if (!strcmp(buf, "exit")) {
            break;
        } else {
            const char* args[] = {"arg1", "arg2arg2", 0};
            auto pid = spawn(buf, args);
            if (pid <= 0) {
                fputs("launch failed\n", stdout);
            } else {
                int stat;
                if (pid == waitpid(pid, &stat, 0)) {
                    if (0 == WEXITSTATUS(stat)) {
                        fputs("return 0\n", stdout);
                    } else {
                        fputs("return none 0\n", stdout);
                    }
                } else {
                    fputs("wait failed\n", stdout);
                }
            }
        }
        fputs("\n> ", stdout);
    }
    return 0;
}
