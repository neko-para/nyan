#include <signal.h>
#include <stdio.h>
#include <unistd.h>

void action(int sig) {
    fputs("action come\n", stderr);
    exit(0);
}

extern "C" int main() {
    signal(SIGUSR1, action);

    while (true) {
        ;
    }

    return 0;
}
