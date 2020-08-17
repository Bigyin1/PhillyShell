file(GLOB source_files "*.c")

add_library(${SHELL_LIB} ${source_files})

target_include_directories(${SHELL_LIB} PUBLIC ../includes)
target_link_libraries(${SHELL_LIB} PRIVATE structs)

