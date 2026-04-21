#include <signal.h>
#include <stdio.h>
#include <unistd.h>

void action(int sig) {
    printf("action come, sig %d\n", sig);
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
