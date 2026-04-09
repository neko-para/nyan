#pragma once

#include <stdint.h>

#include "directory.hpp"
#include "table.hpp"

namespace nyan::paging {

extern Directory kernelPageDirectory;
extern Table kernelPageTable[256];

void clearIdentityPaging();

}  // namespace nyan::paging
