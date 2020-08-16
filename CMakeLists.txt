cmake_minimum_required(VERSION 3.17)
project(PhillyShell C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CFLAGS}")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")

set(SHELL_BIN_NAME fsh)
add_executable(${SHELL_BIN_NAME} main.c)
add_subdirectory(libstructs)
target_link_libraries(${SHELL_BIN_NAME} PUBLIC structs)
