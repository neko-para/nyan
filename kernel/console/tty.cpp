#include "tty.hpp"

#include <nyan/errno.h>
#include <signal.h>

#include "../keyboard/forward.hpp"
#include "../task/mod.hpp"

namespace nyan::console {

void Tty::activate() noexcept {
    __flags |= F_Active;
    flush();
}

void Tty::deactivate() noexcept {
    __flags &= ~F_Active;
}

void Tty::input(const keyboard::Message& msg) noexcept {
    if (msg.flag & keyboard::F_Release) {
        return;
    }

    switch (msg.code) {
        case keyboard::SC_LCTRL:
        case keyboard::SC_RCTRL:
        case keyboard::SC_LSHIFT:
        case keyboard::SC_RSHIFT:
        case keyboard::SC_LALT:
        case keyboard::SC_RALT:
        case keyboard::SC_CAPLOCKS:
        case keyboard::SC_NUMLOCK:
        case keyboard::SC_SCROLLLOCK:
            return;
    }

    auto keys = translateMsg(msg);
    if (keys.empty()) {
        return;
    }

    for (auto ch : keys) {
        processKey(ch);
    }

    __wait_list.wakeOne(task::WakeReason::WR_Normal);
}

bool Tty::inputEmpty() noexcept {
    arch::InterruptGuard guard;
    return __input_buffer.empty();
}

Result<arch::InterruptGuard> Tty::syncWaitInput() noexcept {
    while (true) {
        arch::InterruptGuard guard;
        if (!__input_buffer.empty()) {
            return guard;
        }
        if (__pending_eof) {
            __pending_eof = false;
            return guard;
        }
        if (__wait_list.wait(task::BlockReason::BR_WaitInput) == task::WakeReason::WR_Signal) {
            return SYS_EINTR;
        }
    }
}

std::string Tty::translateMsg(const keyboard::Message& msg) noexcept {
    switch (msg.code) {
        case keyboard::SC_UP:
            return "\x1B[A";
        case keyboard::SC_DOWN:
            return "\x1B[B";
        case keyboard::SC_LEFT:
            return "\x1B[D";
        case keyboard::SC_RIGHT:
            return "\x1B[C";
        case keyboard::SC_HOME:
            return "\x1B[H";
        case keyboard::SC_END:
            return "\x1B[F";
        case keyboard::SC_DELETE:
            return "\x1B[3~";
        case keyboard::SC_INSERT:
            return "\x1B[2~";
        case keyboard::SC_PAGEUP:
            return "\x1B[5~";
        case keyboard::SC_PAGEDOWN:
            return "\x1B[6~";
        case keyboard::SC_F1:
            return "\x1BOP";
        case keyboard::SC_F2:
            return "\x1BOQ";
        case keyboard::SC_F3:
            return "\x1BOR";
        case keyboard::SC_F4:
            return "\x1BOS";
    }

    if (!msg.ch) {
        return "";
    }

    if (msg.flag & keyboard::F_Alt) {
        return {'\x1B', msg.ch};
    } else {
        return {msg.ch};
    }
}

void Tty::processKey(char ch) noexcept {
    if ((__config.c_iflag & ICRNL) && ch == '\r') {
        ch = '\n';
    }

    if (__config.c_lflag & ISIG) {
        if (ch == __config.c_cc[VINTR]) {
            echoCtrl(ch);
            __line_buffer.clear();
            task::kill(-__foreground_pgid, SIGINT) | __ignore;
            return;
        } else if (ch == __config.c_cc[VSUSP]) {
            echoCtrl(ch);
            __line_buffer.clear();
            task::kill(-__foreground_pgid, SIGTSTP) | __ignore;
            return;
        } else if (ch == __config.c_cc[VQUIT]) {
            echoCtrl(ch);
            __line_buffer.clear();
            task::kill(-__foreground_pgid, SIGQUIT) | __ignore;
            return;
        }
    }

    if (__config.c_lflag & ICANON) {
        if (ch == __config.c_cc[VERASE]) {
            if (!__line_buffer.empty()) {
                if (__config.c_lflag & ECHOE) {
                    puts("\b \b", 3);
                }
                __line_buffer.pop_back();
            }
            return;
        } else if (ch == __config.c_cc[VEOF]) {
            __input_buffer.append(__line_buffer);
            __line_buffer.clear();
            __pending_eof = true;
            return;
        } else if (ch == '\n') {
            if (__config.c_lflag & (ECHO | ECHONL)) {
                putc('\n');
            }
            __line_buffer.push_back('\n');
            __input_buffer.append(__line_buffer);
            __line_buffer.clear();
            return;
        }

        echoChar(ch);
        __line_buffer.push_back(ch);
    } else {
        echoChar(ch);
        __input_buffer.push_back(ch);
    }
}

void Tty::echoCtrl(char ch) noexcept {
    if (__config.c_lflag & ECHO) {
        putcImpl('^');
        putcImpl(ch + '@');
        putc('\n');
    }
}

void Tty::echoChar(char ch) noexcept {
    if (__config.c_lflag & ECHO) {
        if ((__config.c_lflag & ECHOCTL) && static_cast<uint8_t>(ch) < 0x20 && ch != '\n' && ch != '\t') {
            putcImpl('^');
            putc(ch + '@');
        } else {
            putc(ch);
        }
    }
    // 不需要检查 ECHONL, 该选项需要 ICANON
}

}  // namespace nyan::console
