#ifndef PHILLYSHELL_SHELL_H
#define PHILLYSHELL_SHELL_H

#include "executor/exec.h"
#include "structs/hashtable.h"
#include "structs/slice.h"
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>

typedef struct s_cfg
{
  char *prompt;

} t_config;

typedef struct s_shell
{
  t_config *cfg;
  char cmd_buf[MAX_INPUT];
  sh_executor e;
} Shell;

#endif // PHILLYSHELL_SHELL_H
