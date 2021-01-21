#include "jobs.h"
#include "../errors/errors.h"
#include "../tty/tty.h"
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>

bool
job_any_running_procs (job *j)
{
  Node *curr = NULL;
  for (list_get_head (j->procs, &curr); curr; curr = curr->next)
    {
      process *p = (process *)curr->data;
      if (!p->completed && !p->stopped)
        return true;
    }
  return false;
}

bool
job_is_stopped (job *j)
{
  Node *curr = NULL;
  for (list_get_head (j->procs, &curr); curr; curr = curr->next)
    {
      process *p = (process *)curr->data;
      if (!p->stopped)
        return false;
    }
  return true;
}

bool
job_is_completed (job *j)
{
  Node *curr = NULL;
  for (list_get_head (j->procs, &curr); curr; curr = curr->next)
    {
      process *p = (process *)curr->data;
      if (!p->completed)
        return false;
    }
  return true;
}

void
update_job (job *j, pid_t pid, int status)
{
  Node *curr = NULL;
  for (list_get_head (j->procs, &curr); curr; curr = curr->next)
    {
      process *p = (process *)curr->data;
      if (p->pid != pid)
        continue;
      p->status = WEXITSTATUS (p->status);
      if (WIFSTOPPED (status))
        p->stopped = true;
      else
        p->completed = true;
      return;
    }
  errors_fatal ("fsh: unexpected job control error\n");
}

void
wait_for_job (job *j, bool no_block, bool interactive)
{
  int options = WSTOPPED;
  if (no_block)
    options |= WNOHANG;
  if (!interactive)
    options = 0; // if not interactive wait only for terminated
  while (job_any_running_procs (j))
    {
      int status;
      pid_t p = waitpid (-j->pgid, &status, options);
      if (p == 0)
        break;
      if (p == -1)
        {
          perror ("fsh");
          errors_fatal ("fsh: fatal waitpid error\n");
        }
      update_job (j, p, status);
    }
}

int
get_job_exit_code (job *j, bool interactive)
{
  wait_for_job (j, false, interactive);

  if (interactive) // restore tty state
    {
      tcsetpgrp (STDIN_FILENO, getpgrp ());
      tcgetattr (STDIN_FILENO, &j->job_term); // save job's termios
      tcsetattr (STDIN_FILENO, TCSADRAIN, &tty_fsh);
    }
  if (job_is_stopped (j) && interactive)
    {
      j->is_background = true;
      printf ("[%d]\tStopped\t%s\n", j->id, j->command);
      return EXIT_SUCCESS;
    }

  Node *n;
  list_get_head (j->procs, &n);
  process *p = n->data;
  return p->status;
}

void
update_bg_jobs (List *jl)
{
  Node *n = NULL;
  for (list_get_head (jl, &n); n; n = n->next)
    {
      job *j = n->data;
      if (!j->is_background)
        continue;
      wait_for_job (j, true, true);
      if (job_is_completed (j))
        printf ("[%d]\tCompleted\n", j->id);
    }
  list_filter_mod (jl, job_delete_func);
}

void
job_set_to_foreground (job *j)
{
  j->is_background = false;
  printf ("%s\n", j->command);
  tcsetpgrp (STDIN_FILENO, j->pgid);
}

void
job_continue (job *j, bool in_fg)
{
  if (in_fg)
    {
      job_set_to_foreground (j);
      tcsetattr (STDIN_FILENO, TCSADRAIN,
                 &j->job_term); // if continue stopped job in foreground,
                                // restore its termios
    }
  kill (-j->pgid, SIGCONT);
  Node *curr = NULL;
  for (list_get_head (j->procs, &curr); curr; curr = curr->next)
    {
      process *p = (process *)curr->data;
      p->stopped = false;
    }
  if (!in_fg)
    printf ("[%d]\t%s &\n", j->id, j->command);
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
new_job (uint id, char *cmd, bool bg, bool subshell)
{
  job *j = calloc (1, sizeof (struct job));
  if (!j)
    errors_fatal (MEM_ERROR);

  if (new_list (&j->procs) != S_OK)
    errors_fatal (MEM_ERROR);

  j->job_term = tty_default;
  j->pgid = -1; // no pgid for now
  if (subshell)
    j->pgid = getpid ();
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
add_new_proc_to_job (job *j, pid_t pid, char *cmd)
{
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
  process *p = calloc (1, sizeof (process));
  if (!p)
    exit (EXIT_FAILURE);
  p->status = status;
  p->completed = true;
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
