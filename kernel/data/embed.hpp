#pragma once

#include <stdint.h>
#include <sys/types.h>

namespace nyan::data {

struct EmbedProgram {
    uint8_t* data;
    size_t size;
    char name[8];
};

extern const EmbedProgram programs[];
extern const size_t programCount;

}  // namespace nyan::data
