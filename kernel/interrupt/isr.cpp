#include "isr.hpp"

#include "entry.hpp"

namespace nyan::interrupt {

template <typename F>
inline void setEntry(Entry& entry, F func) {
    entry = makeEntry(reinterpret_cast<uint32_t>(func), 0x08, A_GateInterrupt | A_Ring0 | A_Present);
}

void fillEntries(Entry* entry) {
    for (size_t i = 0; i < 32; i++) {
        if (i == 3) {
            entry[i] = makeEntry(exceptionStubs[i], 0x08, A_GateInterrupt | A_Ring3 | A_Present);
        } else {
            entry[i] = makeEntry(exceptionStubs[i], 0x08, A_GateInterrupt | A_Ring0 | A_Present);
        }
    }

    setEntry(entry[32], exception_stub_32);
    setEntry(entry[33], exception_stub_33);

    entry[0x80] = makeEntry(reinterpret_cast<uint32_t>(exception_stub_128), 0x08, A_GateTrap | A_Ring3 | A_Present);
}

}  // namespace nyan::interrupt
