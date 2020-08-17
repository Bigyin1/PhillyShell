cmake_minimum_required(VERSION 3.17)
project(PhillyShell C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")

set(SHELL_EXE_NAME fsh)
set(SHELL_LIB shell)
add_executable(${SHELL_EXE_NAME} main.c)

add_subdirectory(libstructs)
add_subdirectory(src)

target_link_libraries(${SHELL_EXE_NAME} ${SHELL_LIB})
