#include "isr.hpp"

#include <stdio.h>

#include "../arch/io.hpp"
#include "../keyboard/load.hpp"
#include "../lib/format.hpp"
#include "../timer/load.hpp"
#include "entry.hpp"
#include "load.hpp"

namespace nyan::interrupt {

template <uint32_t Id>
__attribute__((interrupt)) void defaultHandler(Frame*, uint32_t error) {
    if constexpr (Id == E_PageFault) {
        auto addr = arch::cr2();
        char buf[12] = "0x";
        lib::toCharsHex(buf + 2, addr);
        arch::kputs("Page Fault: ");
        arch::kputs(buf);
        arch::kput('\n');
        if (error & PF_Present) {
            arch::kputs("Present ");
        }
        if (error & PF_Write) {
            arch::kputs("Write ");
        }
        if (error & PF_User) {
            arch::kputs("User ");
        }
        arch::kfatal("");
    } else {
        arch::kfatalfmt("Exception %u: code %u", Id, error);
    }
}

template <uint32_t Id>
__attribute__((interrupt)) void defaultHandlerNe(Frame*) {
    arch::kfatalfmt("Exception %u", Id);
}

__attribute__((interrupt)) void timerHandler(Frame*) {
    end(0);
    timer::hit();
}

__attribute__((interrupt)) void keyboardHandler(Frame*) {
    end(1);
    uint8_t ch = arch::inb(0x60);
    keyboard::push(ch);
}

template <typename F>
inline void setEntry(Entry& entry, F func) {
    entry = makeEntry(reinterpret_cast<uint32_t>(func), 0x08, A_GateInterrupt | A_Ring0 | A_Present);
}

void fillEntries(Entry* entry) {
    setEntry(entry[0], defaultHandlerNe<0>);
    setEntry(entry[1], defaultHandlerNe<1>);
    setEntry(entry[2], defaultHandlerNe<2>);
    setEntry(entry[3], defaultHandlerNe<3>);
    setEntry(entry[4], defaultHandlerNe<4>);
    setEntry(entry[5], defaultHandlerNe<5>);
    setEntry(entry[6], defaultHandlerNe<6>);
    setEntry(entry[7], defaultHandlerNe<7>);
    setEntry(entry[8], defaultHandler<8>);
    setEntry(entry[9], defaultHandlerNe<9>);
    setEntry(entry[10], defaultHandler<10>);
    setEntry(entry[11], defaultHandler<11>);
    setEntry(entry[12], defaultHandler<12>);
    setEntry(entry[13], defaultHandler<13>);
    setEntry(entry[14], defaultHandler<14>);
    setEntry(entry[15], defaultHandlerNe<15>);
    setEntry(entry[16], defaultHandlerNe<16>);
    setEntry(entry[17], defaultHandler<17>);
    setEntry(entry[18], defaultHandlerNe<18>);
    setEntry(entry[19], defaultHandlerNe<19>);
    setEntry(entry[20], defaultHandlerNe<20>);
    setEntry(entry[21], defaultHandler<21>);
    setEntry(entry[22], defaultHandlerNe<22>);
    setEntry(entry[23], defaultHandlerNe<23>);
    setEntry(entry[24], defaultHandlerNe<24>);
    setEntry(entry[25], defaultHandlerNe<25>);
    setEntry(entry[26], defaultHandlerNe<26>);
    setEntry(entry[27], defaultHandlerNe<27>);
    setEntry(entry[28], defaultHandlerNe<28>);
    setEntry(entry[29], defaultHandler<29>);
    setEntry(entry[30], defaultHandler<30>);
    setEntry(entry[31], defaultHandlerNe<31>);

    setEntry(entry[32], timerHandler);
    setEntry(entry[33], keyboardHandler);
}

}  // namespace nyan::interrupt
