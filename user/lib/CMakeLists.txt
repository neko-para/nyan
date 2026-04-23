add_library(
  user STATIC
  src/start.s
  src/libc.cpp
  src/ctype.cpp
  src/errno.cpp
  src/ioctl.cpp
  src/signal.cpp
  src/stdio.cpp
  src/stdlib.cpp
  src/string.cpp
  src/syscall.cpp
  src/unistd.cpp)
target_include_directories(
  user
  PUBLIC ../include
  PRIVATE include)
target_link_libraries(user PUBLIC kernel_header)
