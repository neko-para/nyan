#include <sys/wait.h>
#include <unistd.h>

int strcmp(const char* a, const char* b) {
    while (*a == *b && *a && *b) {
        a++;
        b++;
    }
    return *a - *b;
}

size_t strlen(const char* s) {
    size_t n = 0;
    while (*s++) {
        n++;
    }
    return n;
}

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

void puts(const char* s) {
    write(1, s, strlen(s));
}

extern "C" int main() {
    char buf[256];
    puts("> ");
    while (readline(buf, 255)) {
        if (!strcmp(buf, "exit")) {
            break;
        } else {
            auto pid = spawn(buf);
            if (pid <= 0) {
                puts("launch failed\n");
            } else {
                int stat;
                if (pid == waitpid(pid, &stat, 0)) {
                    if (0 == WEXITSTATUS(stat)) {
                        puts("return 0\n");
                    } else {
                        puts("return none 0\n");
                    }
                } else {
                    puts("wait failed\n");
                }
            }
        }
        puts("\n> ");
    }
    return 0;
}
