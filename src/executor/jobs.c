#include "jobs.h"
#include "../errors/errors.h"
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>


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

bool job_is_stopped(job *j) {
    Node *curr;
    for (list_get_head (j->procs, &curr); curr; curr = curr->next)
    {
        process *p = (process *)curr->data;
        if (!p->stopped)
            return false;
    }
    return true;
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
          p->stopped = true;
      else
          p->completed = true;
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

int
get_job_exit_code (job *j)
{
  while (job_any_running_procs (j)) {
      wait_for_job (j);
      if (job_is_stopped(j)) {
          kill(getpid(), SIGSTOP);
          kill(-j->pgid, SIGCONT);
      }
  }
  Node *n;
  list_get_head (j->procs, &n);
  process *p = n->data;
  return WEXITSTATUS(p->status);
}

job *
new_job ()
{
  job *j = calloc (1, sizeof (struct job));
  if (!j)
    errors_fatal (MEM_ERROR);

  if (new_list (&j->procs) != S_OK)
    errors_fatal (MEM_ERROR);
  return j;
}

bool
job_delete_func (void *p)
{
  job *j = (job *)p;
  if (job_any_running_procs (j))
    return false;

  list_free (j->procs, free);
  free(j);
  return true;
}

void
add_new_proc_to_job (job *j, pid_t pid)
{
  process *p = calloc (1, sizeof (process));
  if (!p)
    exit (EXIT_FAILURE);
  p->pid = pid;
  list_push_back (j->procs, p);
}
