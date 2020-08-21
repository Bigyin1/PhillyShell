set(source_files environ.c shell.c executor/exec_cmd.c errors/errors.c
        parser/pipeline_parser.c
        parser/cmd_parser.c
        parser/utils.c
        parser/parser.c
        tokenizer/cmd_tokenizer.c)

add_library(${SHELL_LIB} ${source_files})

target_include_directories(${SHELL_LIB} PUBLIC ../includes)
target_link_libraries(${SHELL_LIB} PRIVATE structs)

