#include "jobs.h"
#include "../errors/errors.h"
#include "../tty/tty.h"
#include <stdio.h>
#include <sys/types.h>

bool
job_any_running_procs (job *j)
{
  Node *curr = NULL;
  for (list_get_head (j->procs, &curr); curr; curr = curr->next)
    {
      process *p = (process *)curr->data;
      if (!p->exited && !p->stopped && !p->signaled)
        return true;
    }
  return false;
}

bool
job_is_stopped (job *j)
{
  Node *curr = NULL;
  bool one_is_stopped = false;
  for (list_get_head (j->procs, &curr); curr; curr = curr->next)
    {
      process *p = (process *)curr->data;
      if (p->stopped)
        one_is_stopped = true;
      if (!p->exited && !p->stopped && !p->signaled)
        return false;
    }
  if (!one_is_stopped)
    return false;
  return true;
}

bool
job_is_completed (job *j)
{
  Node *curr = NULL;
  for (list_get_head (j->procs, &curr); curr; curr = curr->next)
    {
      process *p = (process *)curr->data;
      if (p->pid == 0)
        continue;
      if (!p->exited && !p->signaled)
        return false;
    }
  return true;
}

int
get_last_job_id (List *jl)
{
  Node *curr = NULL;
  list_get_tail (jl, &curr);
  if (!curr)
    return 0;

  return ((job *)curr->data)->id;
}

job *
new_job (uint id, char *cmd, bool bg, bool interactive)
{
  if (bg && !interactive) /* if job is background and we are not interactive,
                           dont create job at all */
    return NULL;
  job *j = calloc (1, sizeof (struct job));
  if (!j)
    errors_fatal (MEM_ERROR);

  if (new_list (&j->procs) != S_OK)
    errors_fatal (MEM_ERROR);

  j->pgid = -1; // no pgid for now
  if (!interactive)
    j->pgid = getpgrp (); /* if shell is not interactive, there will be no
   dedicated process group for job */
  j->is_background = bg;
  j->id = id;
  j->command = cmd;
  return j;
}

void
proc_delete_func (void *p)
{
  process *proc = (process *)p;
  free (proc->command);
  free (proc);
}

bool
job_delete_func (void *p)
{
  job *j = (job *)p;
  if (!job_is_completed (j))
    return false;

  list_free (j->procs, proc_delete_func);
  free (j->command);
  free (j);
  return true;
}

void
job_delete_force_func (void *p)
{
  job *j = (job *)p;

  list_free (j->procs, proc_delete_func);
  free (j->command);
  free (j);
}

void
add_new_proc_to_job (job *j, pid_t pid, char *cmd)
{
  if (!j)
    return;
  process *p = calloc (1, sizeof (process));
  if (!p)
    exit (EXIT_FAILURE);
  p->pid = pid;
  p->command = cmd;
  list_push_back (j->procs, p);
}

void
add_new_non_fork_proc_to_job (job *j, int status, char *cmd)
{
  if (!j)
    return;
  process *p = calloc (1, sizeof (process));
  if (!p)
    exit (EXIT_FAILURE);
  p->exited = true;
  p->exit_status = status;
  p->command = cmd;
  list_push_back (j->procs, p);
}

void
add_new_job_to_list (List *jl, job *j)
{
  Node *curr = NULL;
  for (list_get_head (jl, &curr); curr; curr = curr->next)
    {
      job *cj = (job *)curr->data;
      if (cj->id == j->id)
        return;
    }
  if (list_push_back (jl, j) != S_OK)
    errors_fatal (MEM_ERROR);
}
