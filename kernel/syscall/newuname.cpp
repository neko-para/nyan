#include <nyan/syscall.h>

#include "../task/mod.hpp"

namespace nyan::syscall {

int newuname(utsname* name) {
    __try
        (task::checkW(name));

    strcpy(name->sysname, "nyan");
    strcpy(name->nodename, "root");
    strcpy(name->release, "nyan");
    strcpy(name->version, "0.0.1");
    strcpy(name->machine, "qemu");
    strcpy(name->domainname, "__domainname");
    return 0;
}

}  // namespace nyan::syscall
