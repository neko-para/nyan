#include <nyan/syscall.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern "C" int main(int argc, char* argv[]) {
    auto str = (char*)malloc(16);
    strcpy(str, "Hello!\n");
    fputs(str, stdout);
    return 0;
}
