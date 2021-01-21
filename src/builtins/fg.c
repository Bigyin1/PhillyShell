//
// Created by sergey on 13.01.2021.
//

#include "../executor/exec.h"
#include "structs/list.h"
#include <signal.h>
#include <sys/types.h>

int
sh_builtin_fg (sh_executor *e, char **argv)
{

  if (!e->bg_fg_enabled) /* set up in exec_pipeline (try to run "fg | command"
                          in bash) */
    {
      fprintf (stderr, "fsh: fg: job control is off\n");
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
        job_continue (j, true);
      else
        job_set_to_foreground (j);

      job_delete_func (
          e->curr_job); /* curr_job has no jobs yet (created for single fg
                         command, which will not be added to this job) and to
                         be deleted and swapped to designated job(job_id) */
      e->curr_job = j;  // swapped
      e->last_jb_id = get_last_job_id (e->active_jobs);
      return EXIT_SUCCESS;
    }

  fprintf (stderr, "fsh: fg: %s: no such task\n", argv[1]);
  return EXIT_FAILURE;
}