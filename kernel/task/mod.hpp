#pragma once

#include "forward.hpp"

namespace nyan::task {

extern Scheduler* __scheduler;

void load() noexcept;

}  // namespace nyan::task