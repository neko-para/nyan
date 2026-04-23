#include "../arch/print.hpp"

extern "C" void __cxa_pure_virtual() {
    nyan::arch::kfatal("pure virtual called");
}
