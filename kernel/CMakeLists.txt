cmake_minimum_required(VERSION 3.30)
project(nyan CXX ASM)
include(../cmake/config.cmake)

file(GLOB user_elfs "${NYAN_USER_INSTALL_DIR}/bin/*")

set(EMBED_OBJECTS "")
set(EMBED_DECL "")
set(EMBED_INIT "")

foreach(elf ${user_elfs})
  get_filename_component(prog_name ${elf} NAME)

  set(BIN_FILE ${CMAKE_BINARY_DIR}/embed/${prog_name}_bin)
  set(OBJ_FILE ${CMAKE_BINARY_DIR}/embed/${prog_name}.o)

  add_custom_command(
    OUTPUT ${OBJ_FILE}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/embed
    COMMAND ${CMAKE_COMMAND} -E copy ${elf} ${BIN_FILE}
    COMMAND
      ${CMAKE_OBJCOPY} -I binary -O elf32-i386 -B i386 --rename-section
      .data=.rodata,alloc,load,readonly,data,contents ${prog_name}_bin
      ${OBJ_FILE}
    DEPENDS ${elf}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/embed
    COMMENT "found user program: ${prog_name}")

  list(APPEND EMBED_OBJECTS ${OBJ_FILE})
  string(APPEND EMBED_DECL "DECLARE_PROGRAM_DATA(${prog_name})\n")
  string(APPEND EMBED_INIT "    DECLARE_PROGRAM(${prog_name}),\n")
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

add_library(kernel_embed STATIC ${CMAKE_BINARY_DIR}/generated/embed.cpp
                                ${EMBED_OBJECTS})
target_include_directories(kernel_embed PRIVATE ${CMAKE_CURRENT_LIST_DIR}/data)

file(GLOB_RECURSE kernel_srcs CONFIGURE_DEPENDS *.cpp *.s)

add_executable(kernel ${kernel_srcs})
target_compile_definitions(kernel PRIVATE __nyan_kernel__)

# 不能开启向量化, 会产生内存引用, 提前启用sse也不行
set_source_files_properties(paging/kernel.cpp PROPERTIES COMPILE_FLAGS
                                                         "-mno-sse")
set_target_properties(kernel PROPERTIES LINK_DEPENDS
                                        ${CMAKE_CURRENT_LIST_DIR}/linker.ld)

target_include_directories(
  kernel SYSTEM BEFORE
  PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include
          ${CMAKE_CURRENT_LIST_DIR}/../sysroot/usr/include/c++/v1)
target_link_directories(kernel PRIVATE
                        ${CMAKE_CURRENT_LIST_DIR}/../sysroot/usr/lib/generic)

target_link_options(kernel PRIVATE -T ${CMAKE_CURRENT_LIST_DIR}/linker.ld)
target_link_libraries(kernel PRIVATE kernel_embed clang_rt.builtins-i386)

install(TARGETS kernel RUNTIME DESTINATION .)
