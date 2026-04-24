#include <nyan/syscall.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

extern "C" int main(int argc, char* argv[]) {
    auto pid = fork();
    sys_write(2, "^^^^^\n", 6);
    if (pid) {
        sys_write(2, "+++++\n", 6);
        fprintf(stdout, "parent process %d, child %d\n", getpid(), pid);
        waitpid(pid, 0, 0);
    } else {
        sys_write(2, "----\n", 6);
        fprintf(stdout, "child process %d\n", getpid());
    }
    sys_write(2, "?????\n", 6);
    return 0;
}
