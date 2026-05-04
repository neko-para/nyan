#include <fcntl.h>
#include <nyan/syscall.h>

#include "../fs/pipe.hpp"
#include "../task/scheduler.hpp"
#include "../task/tcb.hpp"

namespace nyan::syscall {

int pipe(int* fds) {
    if (!fds) {
        return -SYS_EFAULT;
    }

    int readFd, writeFd;

    auto readObjPtr = task::__scheduler->__current->__file.getFileSlot(readFd);
    if (!readObjPtr) {
        return -SYS_EMFILE;
    }

    auto writeObjPtr = task::__scheduler->__current->__file.getFileSlot(writeFd, readFd + 1);
    if (!writeObjPtr) {
        return -SYS_EMFILE;
    }

    auto pipeObj = lib::makeRef<fs::PipeObj>();
    *readObjPtr = lib::makeRef<fs::FdObj>(pipeObj, O_RDONLY);
    *writeObjPtr = lib::makeRef<fs::FdObj>(pipeObj, O_WRONLY);

    fds[0] = readFd;
    fds[1] = writeFd;
    return 0;
}

}  // namespace nyan::syscall
