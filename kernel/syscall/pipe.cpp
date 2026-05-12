#include <fcntl.h>
#include <nyan/syscall.h>

#include "../fs/pipe.hpp"
#include "../task/mod.hpp"
#include "../task/scheduler.hpp"
#include "../task/tcb.hpp"

namespace nyan::syscall {

int pipe(int* fildes) {
    __try
        (task::checkW(fildes, 2));

    int readFd, writeFd;

    auto readObjPtr = __try(task::__scheduler->__current->__file.findFileSlot(readFd));
    auto writeObjPtr = __try(task::__scheduler->__current->__file.findFileSlot(writeFd, readFd + 1));

    auto pipeState = lib::makeRef<fs::PipeState>();
    auto pipeReadObj = lib::makeRef<fs::PipeObj>(pipeState, O_RDONLY);
    auto pipeWriteObj = lib::makeRef<fs::PipeObj>(pipeState, O_WRONLY);
    *readObjPtr = lib::makeRef<fs::FdObj>(pipeReadObj);
    *writeObjPtr = lib::makeRef<fs::FdObj>(pipeWriteObj);

    fildes[0] = readFd;
    fildes[1] = writeFd;
    return 0;
}

}  // namespace nyan::syscall
