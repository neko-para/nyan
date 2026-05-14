#pragma once

#include <string_view>
#include <vector>

#include "../lib/result.hpp"
#include "../lib/shared.hpp"
#include "forward.hpp"
#include "path.hpp"

namespace nyan::fs {

extern std::vector<lib::Ref<MountEntry>>* mountPoints;
extern RamFS* ramFS;

void load();

Result<lib::Ref<FileObj>> open(std::string_view path, uint32_t flags, uint32_t mode = 0644);

struct ResolveConfig {
    bool __follow{true};
    lib::Ref<DEntry> __from{};
};

Result<lib::Ref<DEntry>> resolve(const Path& path, ResolveConfig config, int loop = 0);

}  // namespace nyan::fs
