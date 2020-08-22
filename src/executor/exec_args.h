#ifndef PHILLYSHELL_EXEC_ARGS_H
#define PHILLYSHELL_EXEC_ARGS_H
#include "structs/hashtable.h"
#include "structs/list.h"

char **env_to_kv (HTable *env);
char **args_to_str_arr (char *name, List *args);
void free_string_arr (char **arr);

#endif // PHILLYSHELL_EXEC_ARGS_H
