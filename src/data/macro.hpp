#pragma once

#define DECLARE_PROGRAM_DATA(name)               \
    extern uint8_t _binary_##name##_bin_start[]; \
    extern uint8_t _binary_##name##_bin_end[];

#define DECLARE_PROGRAM(name)                                                                                        \
    {_binary_##name##_bin_start, static_cast<size_t>(&_binary_##name##_bin_end[0] - &_binary_##name##_bin_start[0]), \
     #name}
