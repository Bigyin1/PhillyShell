#ifndef PHILLYSHELL_EXEC_H
#define PHILLYSHELL_EXEC_H
#include "../shell.h"
#include "../parser/parser.h"
//#include "structs/hashtable.h"

typedef struct s_sh_executor {
  char **kv_env;
  HTable *env;
  Slice *path_var;

  int last_retcode;
} sh_executor;

sh_ecode sh_exec (Shell *sh);

#endif // PHILLYSHELL_EXEC_H
