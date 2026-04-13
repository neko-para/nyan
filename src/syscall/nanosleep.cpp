#include "entry.hpp"

#include "../task/task.hpp"

namespace nyan::syscall {

int nanosleep(const timespec* rqtp, timespec* rmtp) {
    std::ignore = rmtp;
    if (!rqtp) {
        return -EFAULT;
    }

    if (rqtp->tv_nsec < 0 || rqtp->tv_nsec >= 1000000000L) {
        return -EINVAL;
    }

    int64_t msec = rqtp->tv_sec * 1000 + rqtp->tv_nsec / 1000000;
    task::sleep(msec);
    return 0;
}

}  // namespace nyan::syscall
