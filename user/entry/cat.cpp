#include <stdio.h>

int main(int argc, char* argv[]) {
    FILE* file = stdin;
    if (argc > 1) {
        file = fopen(argv[1], "rb");
        if (!file) {
            return 1;
        }
    }

    char buf[256];
    while (true) {
        auto sz = fread(buf, 1, 256, file);
        if (sz > 0) {
            fwrite(buf, 1, sz, stdout);
            fflush(stdout);
        } else {
            break;
        }
    }
    return 0;
}
