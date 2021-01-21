#ifndef PHILLYSHELL_EXEC_H
#define PHILLYSHELL_EXEC_H
#include "../parser/parser.h"
#include "jobs.h"
#include <structs/slice.h>

typedef struct s_sh_executor
{
  char **kv_env;
  HTable *env;
  Slice *path_var;

  bool in_subshell;
  // job control
  bool is_interactive;
  bool bg_fg_enabled;

  job *curr_job;
  List *active_jobs;
  uint last_jb_id;
} sh_executor;

sh_ecode execute_cmd (sh_executor *e, char *cmd);

#endif // PHILLYSHELL_EXEC_H
