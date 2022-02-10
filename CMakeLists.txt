cmake_minimum_required(VERSION 3.17)
project(SysImpl)

enable_language(ASM)

set (CMAKE_C_STANDARD 17)
set (CMAKE_CXX_STANDARD 17)

file (GLOB_RECURSE SYSIMPLSRC CONFIGURE_DEPENDS "src/*.c" "src/*.cpp" "src/*.cc" "src/*.s" "src/*.S")

set_source_files_properties("src/intr.cpp" PROPERTIES COMPILE_FLAGS -mgeneral-regs-only)

include_directories (inc)
add_executable (kernel.elf ${SYSIMPLSRC})
target_link_libraries (kernel.elf "gcc")

execute_process( COMMAND ${CMAKE_CXX_COMPILER} -print-file-name=crtbegin.o OUTPUT_VARIABLE CRTBEGIN_O OUTPUT_STRIP_TRAILING_WHITESPACE )
execute_process( COMMAND ${CMAKE_CXX_COMPILER} -print-file-name=crtend.o OUTPUT_VARIABLE CRTEND_O OUTPUT_STRIP_TRAILING_WHITESPACE)

set (CMAKE_CXX_LINK_EXECUTABLE "${tools}gcc <CMAKE_C_LINK_FLAGS> <FLAGS> <LINK_FLAGS> ${CMAKE_SOURCE_DIR}/crt/crti.s ${CRTBEGIN_O} <OBJECTS> -o <TARGET> <LINK_LIBRARIES> ${CRTEND_O} ${CMAKE_SOURCE_DIR}/crt/crtn.s" )
