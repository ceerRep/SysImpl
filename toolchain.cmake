set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR i386)

set (tools i686-elf-)

set (CMAKE_C_COMPILER ${tools}gcc)
set (CMAKE_CXX_COMPILER ${tools}g++)
set (CMAKE_ASM_COMPILER ${tools}gcc)

set (COMMON_FLAGS "-march=i686 -m32 -mgeneral-regs-only -fexceptions -ffreestanding -fno-stack-protector -mno-red-zone -nostdlib -Wno-builtin-declaration-mismatch -Werror=return-type")
set (CMAKE_C_FLAGS "${COMMON_FLAGS}")
set (CMAKE_ASM_FLAGS "${COMMON_FLAGS}")
# set (CMAKE_CXX_FLAGS "${COMMON_FLAGS} -fno-exceptions -fno-rtti -fno-unwind-tables")
set (CMAKE_CXX_FLAGS "${COMMON_FLAGS}")

# set (CMAKE_EXE_LINKER_FLAGS "${COMMON_FLAGS} -Wl,-T${CMAKE_SOURCE_DIR}/ldscript.ld,-melf_i386,-dn")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
