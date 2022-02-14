add_executable(hw.elf hw.c)
target_include_directories(hw.elf PUBLIC "${user_space_inc}")
target_link_libraries(hw.elf userspace-lib)

target_link_options (hw.elf 
    PUBLIC "-Wl,-T${CMAKE_CURRENT_SOURCE_DIR}/hw.ld,-melf_i386,-dn")
