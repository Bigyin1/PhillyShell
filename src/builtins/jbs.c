//
// Created by sergey on 20.12.2020.
//

#include "../executor/exec.h"
#include "structs/list.h"

void
sh_builtin_jobs (sh_executor *e)
{
  Node *curr = NULL;
  for (list_get_head (e->active_jobs, &curr); curr; curr = curr->next)
    {
      job *j = (job *)curr->data;
      if (j->pgid == e->curr_job->pgid)
        continue;
      printf ("[%d] ", j->id);
      if (job_is_stopped (j))
        printf ("Stopped\t");
      else
        printf ("Running\t");
      printf ("%s\n", j->command);
    }
}
