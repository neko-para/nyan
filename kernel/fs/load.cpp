#include "load.hpp"

#include "../data/embed.hpp"
#include "fs/ramfs.hpp"

namespace nyan::fs {

RamFS* ramFS;
SuperBlock* rootSuperBlock;

void load() {
    ramFS = new RamFS;
    rootSuperBlock = ramFS->mount(nullptr, nullptr);

    rootSuperBlock->__root->mkdir("bin", 0755);

    auto bin = rootSuperBlock->__root->lookup("bin");
    for (size_t i = 0; i < data::programCount; i++) {
        const auto& prog = data::programs[i];
        bin->touch(prog.name, 0755);
        auto file = bin->lookup(prog.name);
        file->write(prog.data, prog.size, 0);
    }
}

}  // namespace nyan::fs
