#include <fcntl.h>
#include <nyan/syscall.h>

#include "../fs/pipe.hpp"
#include "../task/tcb.hpp"

namespace nyan::syscall {

int pipe(int* fds) {
    if (!fds) {
        return -SYS_EFAULT;
    }

    int readFd = -1;

    for (size_t i = 3; i < task::MAXFD; i++) {
        auto& fileObjSlot = task::currentTask->fdTable[i];
        if (fileObjSlot) {
            continue;
        }
        if (readFd == -1) {
            readFd = i;
        } else {
            auto pipeObj = lib::makeRef<fs::PipeObj>();
            auto pipeReadObj = lib::makeRef<fs::FdObj>(pipeObj, O_RDONLY);
            auto pipeWriteObj = lib::makeRef<fs::FdObj>(pipeObj, O_WRONLY);

            task::currentTask->fdTable[readFd] = pipeReadObj;
            task::currentTask->fdTable[i] = pipeWriteObj;
            fds[0] = readFd;
            fds[1] = i;
            return 0;
        }
    }
    return -SYS_EMFILE;
}

}  // namespace nyan::syscall
