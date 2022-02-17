add_executable(sleep.elf sleep.c)
target_include_directories(sleep.elf PUBLIC "${user_space_inc}")
target_link_libraries(sleep.elf userspace-lib)

target_link_options (sleep.elf 
    PUBLIC "-Wl,-T${CMAKE_CURRENT_SOURCE_DIR}/sleep.ld,-melf_i386,-dn")
set_target_properties(sleep.elf PROPERTIES POSITION_INDEPENDENT_CODE True)
