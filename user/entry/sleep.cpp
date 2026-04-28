#include <stdlib.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        return 1;
    }
    auto secs = atoi(argv[1]);
    if (secs < 0) {
        return 1;
    }
    sleep(secs);
    return 0;
}
