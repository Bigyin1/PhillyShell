#ifndef PHILLYSHELL_EXEC_H
#define PHILLYSHELL_EXEC_H
#include "../jobs/jobs.h"
#include "../parser/parser.h"
#include <structs/slice.h>

typedef struct s_sh_executor
{
  HTable *env;
  Slice *path_var;

  // job control
  bool is_interactive;
  bool bg_fg_enabled;

  job *curr_job;
  List *active_jobs;
  uint last_jb_id;
} sh_executor;

sh_ecode execute_cmd (sh_executor *e, char *cmd);
void executor_free (sh_executor *e);

#endif // PHILLYSHELL_EXEC_H
