#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

extern "C" int main(int argc, char* argv[]) {
    if (argc == 1) {
        return 1;
    } else {
        pid_t pid = atoi(argv[1]);
        kill(pid, SIGTERM);
        return 0;
    }
}
