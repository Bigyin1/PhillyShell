set(source_files
        environ/environ.c
        shell.c
        executor/exec.c
        executor/fs.c
        executor/exec_args.c
        executor/job_control.c
        errors/errors.c
        parser/pipeline_parser.c
        parser/cmd_parser.c
        parser/utils.c
        parser/parser.c
        parser/parser_free.c
        tokenizer/cmd_tokenizer.c
        builtins/echo.c
        builtins/exit.c
        builtins/cd.c
        builtins/jbs.c
        tty/tty.c)

add_library(${SHELL_LIB} ${source_files})

target_include_directories(${SHELL_LIB} PUBLIC ../includes)
target_link_libraries(${SHELL_LIB} PRIVATE structs)

target_link_libraries(${SHELL_LIB} PRIVATE ncurses)

