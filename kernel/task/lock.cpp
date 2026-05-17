#include "lock.hpp"

#include "scheduler.hpp"

namespace nyan::task {

void SpinLockRaw::lock() noexcept {
    __scheduler->disableYield();
    while (true) {
        if (!__value.exchange(true)) {
            break;
        }
        while (__value.load()) {
            asm volatile("pause");
        }
    }
}

void SpinLockRaw::unlock() noexcept {
    __value.store(false);
    __scheduler->enableYield();
}

}  // namespace nyan::task