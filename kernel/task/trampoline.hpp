#pragma once

#include <stdint.h>
#include <sys/types.h>

namespace nyan::task {

extern uint8_t trampoline_start[] asm("__trampoline_load_start");
extern uint8_t trampoline_end[] asm("__trampoline_load_end");

extern uint8_t sigreturn_trampoline[] asm("__sigreturn_trampoline");

}  // namespace nyan::task
