cmake_minimum_required(VERSION 3.14)
project(SysImpl)

enable_language(ASM)

set (CMAKE_C_STANDARD 11)
set (CMAKE_CXX_STANDARD 17)

file (GLOB_RECURSE SYSIMPLSRC CONFIGURE_DEPENDS "src/*.c" "src/*.cpp" "src/*.cc" "src/*.s" "src/*.S")
file (GLOB BUILTINSRC CONFIGURE_DEPENDS "compile-rt-builtins/*.c")

execute_process( COMMAND ${CMAKE_CXX_COMPILER} -print-file-name=crtbegin.o OUTPUT_VARIABLE CRTBEGIN_O OUTPUT_STRIP_TRAILING_WHITESPACE )
execute_process( COMMAND ${CMAKE_CXX_COMPILER} -print-file-name=crtend.o OUTPUT_VARIABLE CRTEND_O OUTPUT_STRIP_TRAILING_WHITESPACE)

include_directories("common_inc")

add_library (llvm-compile-rt-builtin STATIC ${BUILTINSRC})
target_include_directories (llvm-compile-rt-builtin PUBLIC builtins)

add_executable (kernel.elf ${SYSIMPLSRC})
target_include_directories (kernel.elf PUBLIC inc)
target_link_libraries (kernel.elf "gcc" "llvm-compile-rt-builtin" ${CRTEND_O} "${CMAKE_SOURCE_DIR}/crt/crtn.s")
target_link_options (kernel.elf 
    PUBLIC "-Wl,-T${CMAKE_SOURCE_DIR}/ldscript.ld,-melf_i386,-dn"
    PUBLIC "${CMAKE_SOURCE_DIR}/crt/crti.s"
    PUBLIC ${CRTBEGIN_O})
add_dependencies (kernel.elf llvm-compile-rt-builtin)

add_subdirectory (user)

# set (CMAKE_CXX_LINK_EXECUTABLE "${tools}gcc <CMAKE_C_LINK_FLAGS> <FLAGS> <LINK_FLAGS> ${CMAKE_SOURCE_DIR}/crt/crti.s ${CRTBEGIN_O} <OBJECTS> -o <TARGET> <LINK_LIBRARIES> ${CRTEND_O} ${CMAKE_SOURCE_DIR}/crt/crtn.s" )
