#include <stdio.h>
#include <stdlib.h>

void recursive(int count) {
    int buffer[1000];
    buffer[0] = 0;
    printf("%d %p\n", count, &count);

    if (count > 0) {
        recursive(count - 1);
    }
}

extern "C" int main(int argc, char* argv[]) {
    int rep = atoi(argv[1] ?: "0");
    if (rep < 0) {
        rep = 0;
    }
    recursive(rep);
    return 0;
}
