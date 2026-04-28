set(CMAKE_CXX_LINK_EXECUTABLE
    "<CMAKE_LINKER> <LINK_FLAGS> ${CMAKE_SYSROOT}/usr/lib/crt1.o ${CMAKE_SYSROOT}/usr/lib/crti.o <OBJECTS> ${CMAKE_SYSROOT}/usr/lib/crtn.o -o <TARGET> -L${CMAKE_SYSROOT}/usr/lib -L${CMAKE_SYSROOT}/usr/lib/linux <LINK_LIBRARIES>"
)

file(GLOB user_srcs CONFIGURE_DEPENDS *.cpp)

foreach(user_src ${user_srcs})
  get_filename_component(user_prog_name ${user_src} NAME_WLE)

  add_executable(${user_prog_name} ${user_src})
  target_link_libraries(${user_prog_name} PRIVATE c clang_rt.builtins-i386)
  target_compile_options(${user_prog_name} PRIVATE -Wall -Wextra -g)

  install(TARGETS ${user_prog_name} RUNTIME DESTINATION bin)
endforeach()
