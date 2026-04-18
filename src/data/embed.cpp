#include "embed.hpp"

#define DECLARE_PROGRAM_DATA(name)               \
    extern uint8_t _binary_##name##_bin_start[]; \
    extern uint8_t _binary_##name##_bin_end[];
#define DECLARE_PROGRAM(name)                                                                                        \
    {_binary_##name##_bin_start, static_cast<size_t>(&_binary_##name##_bin_end[0] - &_binary_##name##_bin_start[0]), \
     #name}

DECLARE_PROGRAM_DATA(sh)
DECLARE_PROGRAM_DATA(true)
DECLARE_PROGRAM_DATA(false)
DECLARE_PROGRAM_DATA(echo)

namespace nyan::data {

EmbedProgram programs[4] = {
    DECLARE_PROGRAM(sh),
    DECLARE_PROGRAM(true),
    DECLARE_PROGRAM(false),
    DECLARE_PROGRAM(echo),
};

}
