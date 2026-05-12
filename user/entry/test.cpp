#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int coming_sig;
volatile bool flag = false;

void func(int sig) {
    coming_sig = sig;
    flag = true;
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {
    struct sigaction act{};
    act.sa_handler = func;
    sigaction(SIGINT, &act, 0);
    while (!flag) {
        sleep(5);
    }
    printf("via sig %d\n", coming_sig);
    return 0;
}
