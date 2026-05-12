#include <dirent.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    if (argc > 1) {
        chdir(argv[1]);
    }
    auto dir = opendir(".");
    while (auto info = readdir(dir)) {
        struct stat buf;
        stat(info->d_name, &buf);
        if (S_ISREG(buf.st_mode)) {
            putchar('-');
        } else if (S_ISDIR(buf.st_mode)) {
            putchar('d');
        } else if (S_ISCHR(buf.st_mode)) {
            putchar('c');
        } else if (S_ISBLK(buf.st_mode)) {
            putchar('b');
        } else if (S_ISLNK(buf.st_mode)) {
            putchar('l');
        } else if (S_ISFIFO(buf.st_mode)) {
            putchar('p');
        } else {
            putchar('?');
        }

        putchar((buf.st_mode & S_IRUSR) ? 'r' : '-');
        putchar((buf.st_mode & S_IWUSR) ? 'w' : '-');
        putchar((buf.st_mode & S_IXUSR) ? 'x' : '-');

        putchar((buf.st_mode & S_IRGRP) ? 'r' : '-');
        putchar((buf.st_mode & S_IWGRP) ? 'w' : '-');
        putchar((buf.st_mode & S_IXGRP) ? 'x' : '-');

        putchar((buf.st_mode & S_IROTH) ? 'r' : '-');
        putchar((buf.st_mode & S_IWOTH) ? 'w' : '-');
        putchar((buf.st_mode & S_IXOTH) ? 'x' : '-');

        printf(" %s\n", info->d_name);
    }
    closedir(dir);
    return 0;
}
