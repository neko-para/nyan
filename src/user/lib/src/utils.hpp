#pragma once

#include <errno.h>

template <typename T>
static T wrapRet(T ret) {
    if (ret < 0) {
        errno = -ret;
        return -1;
    } else {
        return ret;
    }
}
