set(USER_PROGRAM_OBJECTS "")
set(USER_PROGRAM_TARGETS "")

set(CMAKE_CXX_LINK_EXECUTABLE
    "<CMAKE_LINKER> <LINK_FLAGS> ${CMAKE_SYSROOT}/usr/lib/crt1.o ${CMAKE_SYSROOT}/usr/lib/crti.o <OBJECTS> ${CMAKE_SYSROOT}/usr/lib/crtn.o -o <TARGET> -L${CMAKE_SYSROOT}/usr/lib -L${CMAKE_SYSROOT}/usr/lib/linux <LINK_LIBRARIES>"
)

function(add_user_program NAME SOURCE)
  add_executable(${NAME} ${SOURCE})
  target_link_libraries(${NAME} PRIVATE c clang_rt.builtins-i386)
  target_compile_options(${NAME} PRIVATE -Wall -Wextra -g)

  set(BIN_FILE ${CMAKE_BINARY_DIR}/${NAME}_bin)
  set(OBJ_FILE ${CMAKE_BINARY_DIR}/${NAME}.o)

  add_custom_command(
    OUTPUT ${OBJ_FILE}
    COMMAND ${CMAKE_COMMAND} ARGS -E copy $<TARGET_FILE:${NAME}> ${BIN_FILE}
    COMMAND
      ${CMAKE_OBJCOPY} ARGS -I binary -O elf32-i386 -B i386 --rename-section
      .data=.rodata,alloc,load,readonly,data,contents ${NAME}_bin ${OBJ_FILE}
    DEPENDS ${NAME}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

  add_custom_target(${NAME}_embed DEPENDS ${OBJ_FILE})

  list(APPEND USER_PROGRAM_OBJECTS ${OBJ_FILE})
  list(APPEND USER_PROGRAM_TARGETS ${NAME}_embed)
  set(USER_PROGRAM_OBJECTS
      "${USER_PROGRAM_OBJECTS}"
      PARENT_SCOPE)
  set(USER_PROGRAM_TARGETS
      "${USER_PROGRAM_TARGETS}"
      PARENT_SCOPE)
endfunction()

file(GLOB user_srcs CONFIGURE_DEPENDS *.cpp)
set(EMBED_DECL "")
set(EMBED_INIT "")

foreach(user_src ${user_srcs})
  get_filename_component(user_prog_name ${user_src} NAME_WLE)
  message("Add user program ${user_src} as ${user_prog_name}")
  add_user_program(${user_prog_name} ${user_src})

  string(APPEND EMBED_DECL "DECLARE_PROGRAM_DATA(${user_prog_name})\n")
  string(APPEND EMBED_INIT "    DECLARE_PROGRAM(${user_prog_name}),\n")
endforeach()

file(
  WRITE ${CMAKE_BINARY_DIR}/generated/embed.cpp
  "#include \"embed.hpp\"
#include \"macro.hpp\"

${EMBED_DECL}
namespace nyan::data {

const EmbedProgram programs[] = {
${EMBED_INIT}};
const size_t programCount = sizeof(programs) / sizeof(programs[0]);

}  // namespace nyan::data
")

add_library(kernel_embed STATIC ${CMAKE_BINARY_DIR}/generated/embed.cpp)
target_link_libraries(kernel_embed PRIVATE kernel_header
                                           ${USER_PROGRAM_OBJECTS})
foreach(target ${USER_PROGRAM_TARGETS})
  add_dependencies(kernel_embed ${target})
endforeach()

target_include_directories(kernel_embed PRIVATE ../../kernel/data)
