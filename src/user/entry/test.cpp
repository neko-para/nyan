#include "../lib/syscall.hpp"

int pid;

extern "C" int main() {
    pid = getpid();
    // const auto& buf = "Hello world!";
    // write(1, buf, sizeof(buf) - 1);
    return 0;
}
