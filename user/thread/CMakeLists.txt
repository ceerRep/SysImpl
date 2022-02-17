add_executable(thread.elf thread.c)
target_include_directories(thread.elf PUBLIC "${user_space_inc}")
target_link_libraries(thread.elf userspace-lib)

target_link_options (thread.elf 
    PUBLIC "-Wl,-T${CMAKE_CURRENT_SOURCE_DIR}/thread.ld,-melf_i386,-dn")
set_target_properties(thread.elf PROPERTIES POSITION_INDEPENDENT_CODE True)
