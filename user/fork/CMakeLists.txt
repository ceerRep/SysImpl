add_executable(fork.elf fork.c)
target_include_directories(fork.elf PUBLIC "${user_space_inc}")
target_link_libraries(fork.elf userspace-lib)

target_link_options (fork.elf 
    PUBLIC "-Wl,-T${CMAKE_CURRENT_SOURCE_DIR}/fork.ld,-melf_i386,-dn")
