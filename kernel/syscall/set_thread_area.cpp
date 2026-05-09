#include <nyan/syscall.h>

#include "../gdt/entry.hpp"
#include "../gdt/load.hpp"
#include "../task/mod.hpp"
#include "../task/scheduler.hpp"
#include "../task/tcb.hpp"

namespace nyan::syscall {

// user_desc flags (第4个uint32_t) 位域定义:
//   bit 0:   seg_32bit
//   bit 1-2: contents (0=data, 1=data expand-down, 2=code non-conforming, 3=code conforming)
//   bit 3:   read_exec_only
//   bit 4:   limit_in_pages
//   bit 5:   seg_not_present
//   bit 6:   useable

static void parseUserDescFlags(uint32_t udFlags, uint8_t& access, uint8_t& flags) {
    bool seg32bit = udFlags & (1 << 0);
    uint8_t contents = (udFlags >> 1) & 0x3;
    bool readExecOnly = udFlags & (1 << 3);
    bool limitInPages = udFlags & (1 << 4);
    bool segNotPresent = udFlags & (1 << 5);
    bool useable = udFlags & (1 << 6);

    // 构造 GDT access byte
    access = gdt::A_Accessed | gdt::A_NotSystem | gdt::A_Ring3;

    if (!segNotPresent) {
        access |= gdt::A_Present;
    }

    if (contents & 0x2) {
        // 代码段 (contents bit 1 set)
        access |= gdt::A_Executable;
        if (contents & 0x1) {
            access |= gdt::A_CodeConforming;
        }
        if (!readExecOnly) {
            access |= gdt::A_CodeReadable;
        }
    } else {
        // 数据段 (contents bit 1 clear)
        if (contents & 0x1) {
            access |= gdt::A_DataDirection;  // expand-down
        }
        if (!readExecOnly) {
            access |= gdt::A_DataWritable;
        }
    }

    // 构造 GDT flags nibble
    flags = 0;
    if (seg32bit) {
        flags |= gdt::F_Size;
    }
    if (limitInPages) {
        flags |= gdt::F_Granularity;
    }
    if (useable) {
        flags |= gdt::F_Long;  // 非64位模式下此位为AVL (OS available), 用户可用
    }
}

int set_thread_area(uint32_t user_desc[4]) {
    __try
        (task::checkW(user_desc, 4));

    auto& entry = user_desc[0];
    if (entry == static_cast<uint32_t>(-1)) {
        entry = 6;
    } else if (entry != 6) {
        return SYS_EINVAL;
    }

    uint8_t access, flags;
    parseUserDescFlags(user_desc[3], access, flags);

    task::__scheduler->__current->tls = gdt::makeSegment(user_desc[1], user_desc[2], access, flags);
    gdt::setTls(task::__scheduler->__current->tls);
    return 0;
}

}  // namespace nyan::syscall
