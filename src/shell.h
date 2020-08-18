#ifndef PHILLYSHELL_SHELL_H
#define PHILLYSHELL_SHELL_H

#include "structs/hashtable.h"
#include "structs/slice.h"
#include <stdbool.h>
#include <stdio.h>
#include <limits.h>

typedef struct s_cfg
{
  char *prompt;

} t_config;

typedef struct s_shell
{
  t_config *cfg;
  HTable *env;
  Slice *path;


  char cmd_buf[MAX_INPUT];
  FILE *out;
  FILE *inp;
  bool running;
} Shell;

typedef enum e_shell_codes
{
  SH_OK = 0,
  SH_MODERATE = 1,
  SH_FATAL = 2,
} sh_err;

#endif // PHILLYSHELL_SHELL_H
