#pragma once

#include <stddef.h>
#include <stdint.h>

namespace nyan::data {

struct EmbedProgram {
    uint8_t* data;
    size_t size;
    char name[8];
};

extern EmbedProgram programs[3];

}  // namespace nyan::data
