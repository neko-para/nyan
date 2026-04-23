#pragma once

#include <nyan/ioctls.h>
#include <sys/types.h>

__NYAN_EXTERNC_BEGIN__

int ioctl(int fd, uint32_t request, ...);

__NYAN_EXTERNC_END__
