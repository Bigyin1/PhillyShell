//
// Created by sergey on 13.01.2021.
//

#include "../executor/exec.h"
#include "structs/list.h"
#include <signal.h>
#include <sys/types.h>

int
sh_builtin_bg (sh_executor *e, char **argv)
{

  if (!e->bg_fg_enabled) /* set up in exec_pipeline (try to run "bg | command"
                          in bash) */
    {
      printf ("fsh: bg: job control is off\n");
      return EXIT_FAILURE;
    }

  uint job_id;
  if (*(argv + 1) == NULL)
    return EXIT_FAILURE;
  job_id = atoi (argv[1]);

  Node *curr;
  for (list_get_head (e->active_jobs, &curr); curr; curr = curr->next)
    {
      job *j = (job *)curr->data;
      if (j->id != job_id)
        continue;
      if (job_is_stopped (j))
        job_continue (j, false);
      else
        {
          printf ("fsh: bg: task %d already running in background\n", job_id);
          return EXIT_FAILURE;
        }
      return EXIT_SUCCESS;
    }

  printf ("fsh: bg: %s: no such task\n", argv[1]);
  return EXIT_FAILURE;
}
