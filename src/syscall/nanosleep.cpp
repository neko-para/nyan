#include <nyan/syscall.h>

#include "../task/task.hpp"

namespace nyan::syscall {

int nanosleep(const timespec* rqtp, [[maybe_unused]] timespec* rmtp) {
    if (!rqtp) {
        return -SYS_EFAULT;
    }

    if (rqtp->tv_nsec < 0 || rqtp->tv_nsec >= 1000000000L) {
        return -SYS_EINVAL;
    }

    int64_t msec = rqtp->tv_sec * 1000 + rqtp->tv_nsec / 1000000;
    if (task::sleep(msec) == task::WakeReason::WR_Signal) {
        return -SYS_EINTR;
    }
    return 0;
}

}  // namespace nyan::syscall
