//
// Created by sergey on 20.12.2020.
//

#include "structs/list.h"
#include "../executor/exec.h"


void sh_builtin_jobs(sh_executor *e) {
  Node *curr;
  for (list_get_head (e->active_jobs, &curr); curr; curr = curr->next)
  {
    job *j = (job *)curr->data;
    if (j->pgid == e->curr_job->pgid) continue;
    printf("[%d] command: %s", j->pgid, j->command);
  }
}
