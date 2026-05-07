#pragma once

#include <string_view>

#include "../lib/result.hpp"
#include "../lib/shared.hpp"
#include "forward.hpp"

namespace nyan::fs {

Result<lib::Ref<FileObj>> open(std::string_view path, uint32_t flags, uint32_t mode = 0644);

}
