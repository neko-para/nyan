#include <nyan/syscall.h>

namespace nyan::syscall {

void exit_group(int code) {
    exit(code);
}

}  // namespace nyan::syscall
