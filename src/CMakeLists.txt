set(source_files
        environ/environ.c
        shell.c
        executor/exec.c
        executor/fs.c
        executor/exec_args.c
        jobs/jobs.c
        jobs/job_control.c
        errors/errors.c
        parser/pipeline_parser.c
        parser/cmd_parser.c
        parser/utils.c
        parser/parser.c
        parser/parser_free.c
        parser/if_list_parser.c
        parser/list_parser.c
        tokenizer/cmd_tokenizer.c
        builtins/echo.c
        builtins/exit.c
        builtins/cd.c
        builtins/jbs.c
        builtins/bg.c
        builtins/fg.c
        tty/tty.c
        tty/readline.c)

add_executable(${SHELL_EXE_NAME} ${source_files})

target_link_libraries(${SHELL_EXE_NAME} structs)
target_link_libraries(${SHELL_EXE_NAME} ncurses)

