#ifndef PHILLYSHELL_ERRORS_H
#define PHILLYSHELL_ERRORS_H


#define PARSE_ERROR "fsh: parse_tokens error at pos: %d\n"
#define MEM_ERROR "fsh: not enough memory, exiting\n"
#define EMPTY_CMD "fsh: command at pos %d is empty\n"
#define UNKNOWN_SYMBOL "fsh: unknown symbol at pos: %d\n"

void errors_fatal (char *err);

typedef enum e_shell_codes
{
  SH_OK = 0,
  SH_ERR = 1,
} sh_ecode;

#endif // PHILLYSHELL_ERRORS_H
