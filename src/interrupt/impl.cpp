#include "isr.hpp"

#include "../arch/io.hpp"
#include "../lib/format.hpp"

namespace nyan::interrupt {

template <uint32_t Id>
void defaultHandlerImpl(Frame*, uint32_t error) {
    if constexpr (Id == E_PageFault) {
        auto addr = arch::cr2();
        char buf[12] = "0x";
        lib::__format::toCharsHex(buf + 2, addr);
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
        arch::kfatal();
    } else {
        arch::kfatal("Exception {}: code {}", Id, error);
    }
}

template <uint32_t Id>
void defaultHandlerImplNe(Frame*) {
    arch::kfatal("Exception {}", Id);
}

template void defaultHandlerImplNe<0>(Frame*);
template void defaultHandlerImplNe<1>(Frame*);
template void defaultHandlerImplNe<2>(Frame*);
template void defaultHandlerImplNe<3>(Frame*);
template void defaultHandlerImplNe<4>(Frame*);
template void defaultHandlerImplNe<5>(Frame*);
template void defaultHandlerImplNe<6>(Frame*);
template void defaultHandlerImplNe<7>(Frame*);
template void defaultHandlerImpl<8>(Frame*, uint32_t);
template void defaultHandlerImplNe<9>(Frame*);
template void defaultHandlerImpl<10>(Frame*, uint32_t);
template void defaultHandlerImpl<11>(Frame*, uint32_t);
template void defaultHandlerImpl<12>(Frame*, uint32_t);
template void defaultHandlerImpl<13>(Frame*, uint32_t);
template void defaultHandlerImpl<14>(Frame*, uint32_t);
template void defaultHandlerImplNe<15>(Frame*);
template void defaultHandlerImplNe<16>(Frame*);
template void defaultHandlerImpl<17>(Frame*, uint32_t);
template void defaultHandlerImplNe<18>(Frame*);
template void defaultHandlerImplNe<19>(Frame*);
template void defaultHandlerImplNe<20>(Frame*);
template void defaultHandlerImpl<21>(Frame*, uint32_t);
template void defaultHandlerImplNe<22>(Frame*);
template void defaultHandlerImplNe<23>(Frame*);
template void defaultHandlerImplNe<24>(Frame*);
template void defaultHandlerImplNe<25>(Frame*);
template void defaultHandlerImplNe<26>(Frame*);
template void defaultHandlerImplNe<27>(Frame*);
template void defaultHandlerImplNe<28>(Frame*);
template void defaultHandlerImpl<29>(Frame*, uint32_t);
template void defaultHandlerImpl<30>(Frame*, uint32_t);
template void defaultHandlerImplNe<31>(Frame*);

}  // namespace nyan::interrupt
