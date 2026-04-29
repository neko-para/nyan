#include "load.hpp"

#include "fs/ramfs.hpp"

namespace nyan::fs {

RamFS* ramFS;
SuperBlock* rootSuperBlock;

void load() {
    ramFS = new RamFS;
    rootSuperBlock = ramFS->mount(nullptr, nullptr);
}

}  // namespace nyan::fs
