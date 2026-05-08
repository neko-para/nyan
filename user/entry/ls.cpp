#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    char path[256];
    if (argc == 1) {
        getcwd(path, 256);
    } else {
        strcpy(path, argv[1]);
    }
    auto dir = opendir(path);
    while (auto info = readdir(dir)) {
        printf("%s\n", info->d_name);
    }
    closedir(dir);
    return 0;
}
