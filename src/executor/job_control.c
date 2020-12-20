#include "job_control.h"
#include "../errors/errors.h"
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>

bool
job_any_running_procs (job *j)
{
  Node *curr;
  for (list_get_head (j->procs, &curr); curr; curr = curr->next)
    {
      process *p = (process *)curr->data;
      if (!p->completed && !p->stopped)
        return true;
    }
  return false;
}

void
update_job (job *j, pid_t pid, int status)
{
  Node *curr;
  for (list_get_head (j->procs, &curr); curr; curr = curr->next)
    {
      process *p = (process *)curr->data;
      if (p->pid != pid)
        continue;
      p->status = status;
      if (WIFSTOPPED (status))
        {
          p->stopped = true;
          fprintf (stderr, "%d: Stopped by signal.\n", (int)pid);
        }
      else
        {
          p->completed = true;
          if (WIFSIGNALED (status))
            fprintf (stderr, "%d: Terminated by signal %d.\n", (int)pid,
                     WTERMSIG (p->status));
        }
      return;
    }
  errors_fatal ("fsh: unexpected job control error\n");
}

void
wait_for_job (job *j)
{
  do
    {
      int status;
      pid_t p = waitpid (WAIT_ANY, &status, WSTOPPED);
      if (errno == ECHILD)
        break;
      if (p == -1)
        errors_fatal ("fsh: fatal waitpid error\n");
      update_job (j, p, status);
    }
  while (job_any_running_procs (j));
  tcsetpgrp (0, getpid ());
}

bool
job_delete_func (void *p)
{
  job *j = (job *)p;
  if (job_any_running_procs (j))
    return false;

  list_free (j->procs, free);
  return true;
}
