#include "../lib/syscall.hpp"

int pid;

extern "C" int main() {
    char buf[256];
    char* ptr = buf;
    while (true) {
        auto count = read(0, ptr, 255 - (ptr - buf));
        if (count <= 0) {
            *ptr = 0;
            break;
        }
        ptr[count] = 0;
        bool quit = false;
        for (ssize_t i = 0; i < count; i++) {
            if (ptr[i] == '\n') {
                quit = true;
                break;
            }
        }
        ptr += count;
        if (quit) {
            break;
        }
    }
    pid = getpid();
    write(1, buf, ptr - buf);
    return 0;
}
