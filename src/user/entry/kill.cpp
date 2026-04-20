#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

extern "C" int main(int argc, char* argv[]) {
    if (argc == 1) {
        return 1;
    } else {
        int arg1 = atoi(argv[1]);
        if (arg1 < 0 && argc >= 2) {
            int arg2 = atoi(argv[2]);
            if (arg2 > 0) {
                return kill(arg2, -arg1);
            } else {
                return 1;
            }
        } else {
            return kill(arg1, SIGTERM);
        }
    }
}
