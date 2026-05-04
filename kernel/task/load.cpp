#include <string.h>

#include "../allocator/mod.hpp"
#include "../paging/directory.hpp"
#include "mod.hpp"
#include "pid.hpp"
#include "scheduler.hpp"
#include "trampoline.hpp"

namespace nyan::task {

lib::Lazy<Scheduler> __scheduler;

static void loadTrampoline() noexcept {
    paging::kernelPageDirectory.set(paging::kernelPageDirectory.at(1023), 1023,
                                    paging::PDE_Present | paging::PDE_ReadWrite | paging::PDE_User);
    paging::kernelPageDirectory.map(0xFFFFF000_va, allocator::physicalFrameAlloc(),
                                    paging::PTE_Present | paging::PTE_ReadWrite | paging::PTE_User);
    0xFFFFF000_va .invlpg();
    void* frame = 0xFFFFF000_va .as<void*>();
    memset(frame, 0, 4096);
    memcpy(frame, trampoline_start, trampoline_end - trampoline_start);
    // TODO: 这里可以配置完成后把readwrite干掉
}

void load() noexcept {
    __scheduler.construct();

    setupKnownTasks();
    loadTrampoline();
}

}  // namespace nyan::task