#include <signal.h>
#include <stdio.h>
#include <unistd.h>

void action(int sig) {
    fputs("action come", stderr);
    exit(sig);
}

extern "C" int main() {
    signal(SIGUSR1, action);

    while (true) {
        ;
    }

    return 0;
}
