#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {
    auto str = (char*)malloc(16);
    strcpy(str, "Hello!\n");
    fputs(str, stdout);
    return 0;
}
