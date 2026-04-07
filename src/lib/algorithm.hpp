#pragma once

#include <stddef.h>

namespace nyan::lib {

template <typename It, typename T>
void fill(It begin, It end, const T& val) {
    while (begin != end) {
        *begin = val;
        begin++;
    }
}

template <typename It, typename T>
void fill_n(It begin, size_t count, const T& val) {
    for (size_t i = 0; i < count; i++) {
        *begin = val;
        begin++;
    }
}

}  // namespace nyan::lib
