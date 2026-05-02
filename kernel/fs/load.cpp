#include "load.hpp"

#include "../data/embed.hpp"
#include "dentry.hpp"
#include "fs/ramfs.hpp"
#include "mount.hpp"

namespace nyan::fs {

std::vector<lib::Ref<MountEntry>>* mountPoints;
RamFS* ramFS;

void load() {
    dentryCacheManager = new DEntryCacheManager;

    mountPoints = new std::vector<lib::Ref<MountEntry>>{};
    ramFS = new RamFS;
    auto rootEntry = ramFS->mount(nullptr, nullptr);
    rootEntry->__mount_point = lib::makeRef<DEntry>();
    rootEntry->__mount_point->__id = allocDEntryId();
    rootEntry->__mount_point->__mount = rootEntry;
    mountPoints->push_back(rootEntry);

    rootEntry->__root_node->mkdir("bin", 0755);
    auto bin = rootEntry->__root_node->lookup("bin");
    for (size_t i = 0; i < data::programCount; i++) {
        const auto& prog = data::programs[i];
        bin->create(prog.name, 0755);
        auto file = bin->lookup(prog.name);
        file->write(prog.data, prog.size, 0);
    }
}

}  // namespace nyan::fs
