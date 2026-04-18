#include "entry.hpp"

namespace nyan::tty {

Tty* activeTty;
Tty allTtys[count];

void Tty::activate() {
    flags |= F_Active;
    flush();
}

void Tty::deactivate() {
    flags &= ~F_Active;
}

void Tty::input(keyboard::Message msg) {
    if (msg.ch) {
        putc(msg.ch);
    }
}

void load() {
    for (auto& tty : allTtys) {
        tty.currentAttr = vga::makeAttr(vga::C_LightGray, vga::C_Black);
        tty.flags = F_ShowCursor;
    }
    activeTty = &allTtys[0];
    activeTty->clear();
    activeTty->activate();
}

}  // namespace nyan::tty
