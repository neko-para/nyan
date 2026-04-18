#include <sys/types.h>

static int __errno;

extern "C" int* __llvm_libc_errno() {
    return &__errno;
}
