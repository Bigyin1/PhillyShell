#ifndef PHILLYSHELL_BUILTINS_H
#define PHILLYSHELL_BUILTINS_H

#include "structs/hashtable.h"

int sh_builtin_cd (char **argv, HTable *env);
void sh_builtin_echo (char **argv);
void sh_builtin_exit ();

#define CMD_CD "cd"
#define CMD_ECHO "echo"
#define CMD_EXIT "exit"

#endif // PHILLYSHELL_BUILTINS_H
