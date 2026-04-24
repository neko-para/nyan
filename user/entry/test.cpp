#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

extern "C" int main(int argc, char* argv[]) {
    execve("echo", argv, 0);
    return 0;
}
