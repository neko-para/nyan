#include "isr.hpp"

#include "../arch/io.hpp"
#include "../lib/format.hpp"

namespace nyan::interrupt {

template <uint32_t Id>
void defaultHandlerImpl(Frame*, uint32_t error) {
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

template void defaultHandlerImpl<8>(Frame*, uint32_t);
template void defaultHandlerImpl<10>(Frame*, uint32_t);
template void defaultHandlerImpl<11>(Frame*, uint32_t);
template void defaultHandlerImpl<12>(Frame*, uint32_t);
template void defaultHandlerImpl<13>(Frame*, uint32_t);
template void defaultHandlerImpl<14>(Frame*, uint32_t);
template void defaultHandlerImpl<17>(Frame*, uint32_t);
template void defaultHandlerImpl<21>(Frame*, uint32_t);
template void defaultHandlerImpl<29>(Frame*, uint32_t);
template void defaultHandlerImpl<30>(Frame*, uint32_t);

}  // namespace nyan::interrupt
