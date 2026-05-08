file(GLOB user_srcs CONFIGURE_DEPENDS *.cpp)

foreach(user_src ${user_srcs})
  get_filename_component(user_prog_name ${user_src} NAME_WLE)

  add_executable(${user_prog_name} ${user_src})
  target_compile_definitions(${user_prog_name} PRIVATE __nyan__)

  target_compile_options(${user_prog_name} PRIVATE -ffunction-sections
                                                   -fdata-sections)
  target_link_options(${user_prog_name} PRIVATE -Wl,--gc-sections)

  install(TARGETS ${user_prog_name} RUNTIME DESTINATION bin)
endforeach()

file(
  GLOB user_subdirs
  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  *)
foreach(user_subdir ${user_subdirs})
  if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${user_subdir})
    file(GLOB subdir_srcs CONFIGURE_DEPENDS ${user_subdir}/*.cpp)

    if(subdir_srcs)
      add_executable(${user_subdir} ${subdir_srcs})
      target_compile_definitions(${user_subdir} PRIVATE __nyan__)
      target_include_directories(
        ${user_subdir} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/${user_subdir})

      target_compile_options(${user_subdir} PRIVATE -ffunction-sections
                                                    -fdata-sections)
      target_link_options(${user_subdir} PRIVATE -Wl,--gc-sections)

      install(TARGETS ${user_subdir} RUNTIME DESTINATION bin)
    endif()
  endif()
endforeach()
