#include <errno.h>

int __errno;

extern "C" int* __errno_location() {
    return &__errno;
}
