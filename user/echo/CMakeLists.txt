add_executable(echo.elf echo.c)
target_include_directories(echo.elf PUBLIC "${user_space_inc}")
target_link_libraries(echo.elf userspace-lib)

target_link_options (echo.elf 
    PUBLIC "-Wl,-T${CMAKE_CURRENT_SOURCE_DIR}/echo.ld,-melf_i386,-dn")
