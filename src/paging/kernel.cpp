#include "kernel.hpp"

namespace nyan::paging {

Directory kernelPageDirectory;
Table kernelPageTable[256];

}  // namespace nyan::paging
