#pragma once

#include <stdint.h>

namespace nyan::interrupt {

struct Entry;

struct Frame {
    uint32_t ip;
    uint32_t cs;
    uint32_t flags;
    uint32_t sp;
    uint32_t ss;
};

template <uint32_t Id>
__attribute__((interrupt)) void defaultHandler(Frame*, uint32_t error);

template <uint32_t Id>
__attribute__((interrupt)) void defaultHandlerNe(Frame*);

void fillEntries(Entry* entry);

}  // namespace nyan::interrupt
