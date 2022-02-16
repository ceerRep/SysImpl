add_executable(sh.elf sh.c)
target_include_directories(sh.elf PUBLIC "${user_space_inc}")
target_link_libraries(sh.elf userspace-lib)

target_link_options (sh.elf 
    PUBLIC "-Wl,-T${CMAKE_CURRENT_SOURCE_DIR}/sh.ld,-melf_i386,-dn")
