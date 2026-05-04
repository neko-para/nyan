#pragma once

#include "../logger/print.hpp"
#include "instr.hpp"
#include "port.hpp"

namespace nyan::arch {

inline void qemuQuit() {
    outw(0x604, 0x2000);
}

[[noreturn]] inline void kfatal(void* eip = nullptr) {
    logger::emitFatal(eip);
    cli();
    for (;;) {
        hlt();
    }
}

inline void kwrite(const void* data, size_t len) {
    auto ptr = reinterpret_cast<const uint8_t*>(data);
    while (len--) {
        outb(0xe9, *ptr++);
    }
}

}  // namespace nyan::arch
