#include "message.hpp"

namespace nyan::keyboard {

static const char SC_to_char[] =
    "\0\0\0\0001!2@3#4$5%6^7&8*9(0)-_=+\b\b"
    "\t\tqQwWeErRtTyYuUiIoOpP[{]}\n\n"
    "\0\0aAsSdDfFgGhHjJkKlL;:'\"`~\0\0\\|"
    "zZxXcCvVbBnNmM,<.>/?\0\0**"
    "\0\0  \0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
    "778899--445566++11223300..\0";

uint8_t flags = 0;

Message merge(uint16_t scan) {
    Message msg;
    msg.code = scan;
    msg.key = scan & (~F_Release);
    msg.flag = 0;
    msg.ch = 0;

    if (scan & F_Release) {
        msg.flag |= F_Release;
        scan -= F_Release;
    }

    if ((scan >> 8) == 0xE0) {
        if (scan == SC_KP_SLASH) {
            msg.ch = '/';
        }
    } else if (scan < sizeof(SC_to_char) / 2) {
        msg.ch = SC_to_char[(scan << 1) | ((flags & F_Shift) ? 1 : 0)];
    }

    uint8_t mod = 0;
    switch (scan) {
        case SC_LCTRL:
        case SC_RCTRL:
            mod = F_Ctrl;
            break;
        case SC_LSHIFT:
        case SC_RSHIFT:
            mod = F_Shift;
            break;
        case SC_LALT:
        case SC_RALT:
            mod = F_Alt;
            break;
    }
    if (mod) {
        if (msg.flag & F_Release) {
            flags &= ~mod;
        } else {
            flags |= mod;
        }
    }
    msg.flag |= flags;

    return msg;
}

}  // namespace nyan::keyboard
