#include "../console/device.hpp"
#include "../console/mod.hpp"
#include "../data/embed.hpp"
#include "../lib/format.hpp"
#include "dentry.hpp"
#include "fs/ramfs.hpp"
#include "mod.hpp"
#include "mount.hpp"

namespace nyan::fs {

std::vector<lib::Ref<MountEntry>>* mountPoints;
RamFS* ramFS;

static void loadInitFs() {
    auto entry = rootEntry();

    entry->__root_node->mkdir("bin", 0755) | __ignore;
    auto bin = entry->__root_node->lookup("bin") | __unwrap;
    for (size_t i = 0; i < data::programCount; i++) {
        const auto& prog = data::programs[i];
        bin->create(prog.name, 0755) | __ignore;
        auto file = bin->lookup(prog.name) | __unwrap;
        file->write(prog.data, prog.size, 0) | __ignore;
    }

    entry->__root_node->mkdir("dev", 0755) | __ignore;
    auto dev = entry->__root_node->lookup("dev") | __unwrap;
    for (size_t i = 0; i < console::__tty_count; i++) {
        auto name = lib::format("tty{}", i);

        dev->link(name, lib::makeRef<RamFSCharDevVNode>(console::__all_tty_devices[i], dev->__super_block, 0644)) |
            __ignore;
    }

    entry->__root_node->create("hello", 0755) | __ignore;
    auto hello = entry->__root_node->lookup("hello") | __unwrap;
    hello->write("Hello world!", 12, 0) | __ignore;
}

void load() {
    dentryCacheManager = new DEntryCacheManager;

    mountPoints = new std::vector<lib::Ref<MountEntry>>{};
    ramFS = new RamFS;
    auto entry = ramFS->mount(nullptr, nullptr);
    entry->__mount_point = lib::makeRef<DEntry>();
    entry->__mount_point->__id = allocDEntryId();
    entry->__mount_point->__mount = entry;
    mountPoints->push_back(entry);

    loadInitFs();
}

}  // namespace nyan::fs
