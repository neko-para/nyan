#pragma once

#include <sys/types.h>

namespace nyan::data {

struct EmbedProgram {
    uint8_t* data;
    size_t size;
    char name[8];
};

extern EmbedProgram programs[5];

}  // namespace nyan::data
