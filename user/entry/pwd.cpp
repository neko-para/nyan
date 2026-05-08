#include <stdio.h>
#include <unistd.h>

int main() {
    char path[256];
    getcwd(path, 256);
    puts(path);
    return 0;
}
