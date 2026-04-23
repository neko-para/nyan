#include <stdio.h>

extern "C" int main() {
    // TODO: use fread
    char buf[256];
    while (fgets(buf, 256, stdin)) {
        fputs(buf, stdout);
    }
    return 0;
}
