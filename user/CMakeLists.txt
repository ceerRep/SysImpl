set (user_space_inc "${CMAKE_CURRENT_SOURCE_DIR}/inc")

file (GLOB USERSPACE_LIB_SRC CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/lib/*.c" "${CMAKE_CURRENT_SOURCE_DIR}/lib/*.S")

add_library(userspace-lib ${USERSPACE_LIB_SRC})
target_include_directories(userspace-lib PUBLIC "${user_space_inc}")
target_include_directories(userspace-lib PUBLIC "${CMAKE_SOURCE_DIR}/inc/syscall")
target_link_libraries(userspace-lib llvm-compile-rt-builtin)

add_subdirectory(sh)
add_subdirectory(hlt)
add_subdirectory(echo)
