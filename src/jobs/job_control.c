#include "../errors/errors.h"
#include "../tty/tty.h"
#include "jobs.h"
#include <signal.h>
#include <stdio.h>
#include <sys/wait.h>

#define WAIT_BLOCKING 0
#define WAIT_NON_BLOCKING 1

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

/* get_job_exit_code wait in blocking way for job to not have any running
 * processes and returns exit code of first process of job. If job was stopped,
 * notifies user an return success exit code. */
int
get_job_exit_code (job *j, bool interactive)
{
  wait_for_job (j, WAIT_BLOCKING, interactive);

  if (interactive) // restore tty state
    {
      tcsetpgrp (STDIN_FILENO, getpgrp ());
      tcgetattr (STDIN_FILENO, &j->job_term); // save job's termios
      tty_restore();
    }
  if (job_is_stopped (j) && interactive)
    {
      printf ("[%d]\tStopped\t%s\n", j->id, j->command);
      return EXIT_SUCCESS;
    }

  Node *n;
  list_get_head (j->procs, &n);
  process *p = n->data;
  return p->status;
}

// update_bg_jobs updates state of background jobs in non-blocking way
void
update_bg_jobs (List *jl)
{
  Node *n = NULL;
  for (list_get_head (jl, &n); n; n = n->next)
    {
      job *j = n->data;
      if (!j->is_background)
        continue;
      wait_for_job (j, WAIT_NON_BLOCKING, true);
      if (job_is_completed (j))
        printf ("[%d]\tCompleted\n", j->id);
    }
  list_filter_mod (jl, job_delete_func);
}

// Methods used in bg and fg builtins

void
job_set_to_foreground (job *j)
{
  j->is_background = false;
  printf ("%s\n",
          j->command); // notify user about continuing job in foreground
  tcsetpgrp (STDIN_FILENO,
             j->pgid); /* restoring only tty's process group, termios will be
                        restored only when job is waken up after being
                        stopped (in job_continue) */
}

// continue stopped job
void
job_continue (job *j, bool in_foreground)
{
  if (in_foreground)
    {
      j->is_background = false;
      printf ("%s\n", j->command);
      tcsetpgrp (STDIN_FILENO, j->pgid);
      tcsetattr (STDIN_FILENO, TCSADRAIN,
                 &j->job_term); /* if continuing stopped job in foreground,
       restore its termios */
    }
  else
    {
      j->is_background = true;
      printf ("[%d]\t%s &\n", j->id,
              j->command); // notify user about continuing job in background
    }
  if (kill (-j->pgid, SIGCONT) < 0)
    {
      perror ("fsh:");
      exit (EXIT_FAILURE);
    }
  Node *curr = NULL;
  for (list_get_head (j->procs, &curr); curr; curr = curr->next)
    {
      process *p = (process *)curr->data;
      p->stopped = false;
    }
}
