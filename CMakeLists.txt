cmake_minimum_required(VERSION 3.17)
project(PhillyShell C)

set(CMAKE_C_STANDARD 11)
set(SHELL_EXE_NAME fsh)
set(SHELL_LIB shell)

if(${CMAKE_BUILD_TYPE} STREQUAL Debug)
    add_compile_options("SHELL:-fno-omit-frame-pointer" "SHELL:-fsanitize=address")
    add_link_options("SHELL:-fsanitize=address")
endif()

add_executable(${SHELL_EXE_NAME} main.c)
add_subdirectory(libstructs)

add_subdirectory(src)

target_link_libraries(${SHELL_EXE_NAME} ${SHELL_LIB})
