#include <nyan/syscall.h>

namespace nyan::syscall {

void exit_group(int error_code) {
    exit(error_code);
}

}  // namespace nyan::syscall
