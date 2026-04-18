#pragma once

#include "../keyboard/message.hpp"
#include "../lib/queue.hpp"
#include "../task/wait.hpp"
#include "buffer.hpp"

namespace nyan::tty {

constexpr size_t count = 2;

struct Tty : public ScreenBuffer {
    pid_t currentPid;

    lib::string lineBuffer;

    // TODO: 看看要不改下，不然一行只能输入256了
    lib::RingQueue<uint8_t, 256> inputBuffer;
    task::WaitList waitList;

    void activate();
    void deactivate();

    void input(keyboard::Message msg);

    bool inputEmpty();
    void syncWaitInput();
};

extern Tty* activeTty;
extern Tty allTtys[count];

void load();
void switchTo(Tty* tty);

bool startShellOn(Tty* tty);

}  // namespace nyan::tty
