#include <stdio.h>

extern "C" int main(int argc, char* argv[]) {
    for (int i = 0; i < argc; i++) {
        fputs(argv[i], stdout);
        fputs("\n", stdout);
    }
    return 0;
}
