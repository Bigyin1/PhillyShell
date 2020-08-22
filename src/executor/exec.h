#ifndef PHILLYSHELL_EXEC_H
#define PHILLYSHELL_EXEC_H
#include "../shell.h"
#include "../parser/parser.h"

typedef struct s_sh_executor {
  char **kv_env;
  Slice *path_var;
  sh_ast_node root_node;
} sh_executor;

sh_ecode sh_exec (Shell *sh);

#endif // PHILLYSHELL_EXEC_H
