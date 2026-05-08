#pragma once

#include <string_view>
#include <vector>

#include "../lib/result.hpp"
#include "../lib/shared.hpp"
#include "forward.hpp"

namespace nyan::fs {

extern std::vector<lib::Ref<MountEntry>>* mountPoints;
extern RamFS* ramFS;

void load();

Result<lib::Ref<FileObj>> open(std::string_view path, uint32_t flags, uint32_t mode = 0644);

Result<lib::Ref<DEntry>> resolve(std::string_view path);
Result<lib::Ref<DEntry>> resolveParent(std::string_view path, lib::Ref<DEntry>* parent, std::string* lastName);

inline auto rootEntry() {
    return (*mountPoints)[0];
}

}  // namespace nyan::fs
