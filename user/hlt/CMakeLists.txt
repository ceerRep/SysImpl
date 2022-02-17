add_executable(hlt.elf hlt.c)
target_include_directories(hlt.elf PUBLIC "${user_space_inc}")
target_link_libraries(hlt.elf userspace-lib)

target_link_options (hlt.elf 
    PUBLIC "-Wl,-T${CMAKE_CURRENT_SOURCE_DIR}/hlt.ld,-melf_i386,-dn")
set_target_properties(hlt.elf PROPERTIES POSITION_INDEPENDENT_CODE True)
