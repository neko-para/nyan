#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {
    if (argc > 1) {
        auto file = fopen(argv[1], "wb");
        if (!file) {
            return 1;
        }
        fputs(argv[1], file);
        fflush(file);
        return 0;
    } else {
        return 1;
    }
}
