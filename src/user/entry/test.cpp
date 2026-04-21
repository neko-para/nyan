#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void action(int sig) {
    printf("action come, sig %d    ", sig);
    // _Exit(0);
    exit(0);
}

extern "C" int main() {
    signal(SIGUSR1, action);
    signal(SIGINT, action);

    while (true) {
        ;
    }

    return 0;
}
