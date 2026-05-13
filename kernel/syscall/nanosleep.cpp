#include <nyan/syscall.h>

#include "../arch/print.hpp"
#include "../task/mod.hpp"
#include "../task/scheduler.hpp"

namespace nyan::syscall {

int nanosleep(const timespec* rqtp, timespec* rmtp) {
    __try
        (task::checkR(rqtp));
    __try
        (task::checkW(rmtp, 1, true));

    arch::kprint("sec {#010x} nsec {#010x}", rqtp->tv_sec, rqtp->tv_nsec);

    if (rqtp->tv_sec < 0 || rqtp->tv_nsec < 0 || rqtp->tv_nsec >= 1000000000L) {
        return SYS_EINVAL;
    }

    int64_t msec = rqtp->tv_sec * 1000 + rqtp->tv_nsec / 1000000;
    uint64_t rest = 0;
    int ret = task::__scheduler->sleep(msec, &rest) == task::WakeReason::WR_Signal ? SYS_EINTR : 0;
    if (rmtp) {
        rmtp->tv_sec = rest / 1000;
        rmtp->tv_nsec = (rest % 1000) * 1000000;
    }
    return ret;
}

}  // namespace nyan::syscall
