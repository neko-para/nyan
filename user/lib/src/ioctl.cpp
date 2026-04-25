#include <nyan/syscall.h>
#include <stdarg.h>
#include <sys/ioctl.h>

#include "utils.hpp"

using namespace nyan::user;

extern "C" {

int ioctl(int fd, int request, ...) {
    switch (request) {
        // case TIOCGPGRP:
        case TIOCSPGRP: {
            va_list lst;
            va_start(lst, request);
            return wrapRet(sys_ioctl(fd, request, va_arg(lst, uint32_t)));
        }
    }
    return wrapRet(-ENOSYS);
}
}
