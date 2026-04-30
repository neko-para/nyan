#include "load.hpp"

#include <algorithm>
#include <ranges>
#include <vector>

#include "../arch/print.hpp"
#include "../data/embed.hpp"
#include "fs/ramfs.hpp"
#include "mount.hpp"

namespace nyan::fs {

std::vector<MountEntry>* mountPoints;
RamFS* ramFS;
SuperBlock* rootSuperBlock;

void load() {
    mountPoints = new std::vector<MountEntry>{};
    ramFS = new RamFS;
    rootSuperBlock = ramFS->mount(nullptr, nullptr);

    rootSuperBlock->__root->mkdir("bin", 0755);

    auto bin = rootSuperBlock->__root->lookup("bin");
    for (size_t i = 0; i < data::programCount; i++) {
        const auto& prog = data::programs[i];
        bin->create(prog.name, 0755);
        auto file = bin->lookup(prog.name);
        file->write(prog.data, prog.size, 0);
    }
}

lib::Ref<VNode> resolve(std::string_view path) {
    if (path[0] != '/') {
        // TODO: via current task cwd
        arch::kfatal("relative path not supported yet");
    }

    auto current = rootSuperBlock->__root;
    for (const auto& item : path | std::views::split('/')) {
        std::string_view portion{item.begin(), item.end()};
        if (portion.empty()) {
            continue;
        } else if (portion == ".") {
            continue;
        } else if (portion == "..") {
            // TODO
            arch::kfatal("goto parent not supported");
        } else {
            auto next = current->lookup(portion);
            if (!next) {
                return {};
            }
            if (auto mp = std::find_if(mountPoints->begin(), mountPoints->end(),
                                       [&](const MountEntry& entry) noexcept { return entry.__mount_point == next; });
                mp != mountPoints->end()) {
                next = mp->__super_block->__root;
            }
            current = next;
        }
    }
    return current;
}

}  // namespace nyan::fs
