file(GLOB user_srcs CONFIGURE_DEPENDS *.cpp)

foreach(user_src ${user_srcs})
  get_filename_component(user_prog_name ${user_src} NAME_WLE)

  add_executable(${user_prog_name} ${user_src})

  target_compile_options(${user_prog_name} PRIVATE -ffunction-sections
                                                   -fdata-sections)
  target_link_options(${user_prog_name} PRIVATE -Wl,--gc-sections)

  install(TARGETS ${user_prog_name} RUNTIME DESTINATION bin)
endforeach()
