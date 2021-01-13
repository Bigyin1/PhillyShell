#ifndef PHILLYSHELL_BUILTINS_H
#define PHILLYSHELL_BUILTINS_H

#include "../executor/exec.h"
#include "structs/hashtable.h"

int sh_builtin_cd (char **argv, HTable *env);
void sh_builtin_echo (char **argv);
void sh_builtin_exit ();
void sh_builtin_jobs (sh_executor *e);
int sh_builtin_bg (sh_executor *e, char **argv);
int sh_builtin_fg (sh_executor *e, char **argv);

#define CMD_CD "cd"
#define CMD_ECHO "echo"
#define CMD_EXIT "exit"
#define CMD_JOBS "jobs"
#define CMD_BG "bg"
#define CMD_FG "fg"

#endif // PHILLYSHELL_BUILTINS_H
