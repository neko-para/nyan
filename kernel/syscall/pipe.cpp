#include <fcntl.h>
#include <nyan/syscall.h>

#include "../fs/pipe.hpp"
#include "../task/scheduler.hpp"
#include "../task/tcb.hpp"
#include "utils.hpp"

namespace nyan::syscall {

int pipe(int* fds) {
    if (!utils::validateWriteAuto(fds, 2)) {
        return -SYS_EFAULT;
    }

    int readFd, writeFd;

    auto readObjPtr = task::__scheduler->__current->__file.findFileSlot(readFd);
    if (!readObjPtr) {
        return -SYS_EMFILE;
    }

    auto writeObjPtr = task::__scheduler->__current->__file.findFileSlot(writeFd, readFd + 1);
    if (!writeObjPtr) {
        return -SYS_EMFILE;
    }

    auto pipeState = lib::makeRef<fs::PipeState>();
    auto pipeReadObj = lib::makeRef<fs::PipeObj>(pipeState, O_RDONLY);
    auto pipeWriteObj = lib::makeRef<fs::PipeObj>(pipeState, O_WRONLY);
    *readObjPtr = lib::makeRef<fs::FdObj>(pipeReadObj);
    *writeObjPtr = lib::makeRef<fs::FdObj>(pipeWriteObj);

    fds[0] = readFd;
    fds[1] = writeFd;
    return 0;
}

}  // namespace nyan::syscall
