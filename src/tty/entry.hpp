#pragma once

#include "../keyboard/message.hpp"
#include "buffer.hpp"

namespace nyan::tty {

constexpr size_t count = 2;

struct Tty : public ScreenBuffer {
    ScreenBuffer* buffer;

    void activate();
    void deactivate();

    void input(keyboard::Message msg);
};

extern Tty* activeTty;
extern Tty allTtys[count];

void load();

}  // namespace nyan::tty
