#include "load.hpp"

#include "../console/device.hpp"
#include "../console/mod.hpp"
#include "../data/embed.hpp"
#include "../lib/format.hpp"
#include "dentry.hpp"
#include "fs/ramfs.hpp"
#include "mount.hpp"

namespace nyan::fs {

std::vector<lib::Ref<MountEntry>>* mountPoints;
RamFS* ramFS;

static void loadInitFs() {
    auto entry = rootEntry();

    entry->__root_node->mkdir("bin", 0755);
    auto bin = entry->__root_node->lookup("bin");
    for (size_t i = 0; i < data::programCount; i++) {
        const auto& prog = data::programs[i];
        bin->create(prog.name, 0755);
        auto file = bin->lookup(prog.name);
        file->write(prog.data, prog.size, 0);
    }

    entry->__root_node->mkdir("dev", 0755);
    auto dev = entry->__root_node->lookup("dev");
    for (size_t i = 0; i < console::__tty_count; i++) {
        auto name = lib::format("tty{}", i);
        dev->link(name, lib::makeRef<RamFSCharDevVNode>(console::__all_tty_devices[i], dev->__super_block, 0755));
    }

    entry->__root_node->create("hello", 0755);
    auto hello = entry->__root_node->lookup("hello");
    hello->write("Hello world!", 12, 0);
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
