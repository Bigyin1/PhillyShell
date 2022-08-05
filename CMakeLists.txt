cmake_minimum_required(VERSION 3.17)
project(PhillyShell C)

set(CMAKE_C_STANDARD 11)
set(SHELL_EXE_NAME fsh)

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")

add_subdirectory(libstructs)
add_subdirectory(src)

enable_testing()
add_test(NAME fsh_test
        COMMAND python ${PROJECT_SOURCE_DIR}/tests/fsh_test.py
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(fsh_test PROPERTIES ENVIRONMENT FSH=./src/${SHELL_EXE_NAME})