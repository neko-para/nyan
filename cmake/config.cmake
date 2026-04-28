set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(
  -Wall
  -Wextra
  -Wshadow
  -Wtype-limits
  -Wsign-compare
  -Wcast-align
  -Wnull-dereference)

if(NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "Debug")
  # add_compile_options(-g)
endif()

if(NYAN_ENABLE_OPTIMIZE)
  add_compile_options(-flto=thin -funified-lto -O2 -fomit-frame-pointer
                      -ffunction-sections -fdata-sections)
  add_link_options(--lto=thin --gc-sections)
else()
  add_compile_options(-Og)
endif()
