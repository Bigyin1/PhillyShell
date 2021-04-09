#include "../errors/errors.h"
#include "../tty/tty.h"
#include "jobs.h"
#include <signal.h>
#include <stdio.h>
#include <sys/wait.h>

#define WAIT_BLOCKING 0
#define WAIT_NON_BLOCKING 1

void
proc_report_signaled (process *p, int status, int j_id)
{
  if (WTERMSIG (status) == SIGSEGV)
    {
      fprintf (stderr, "[%d]\t%d segmentation fault ", j_id, p->pid);
      if (WCOREDUMP (status))
        fprintf (stderr, "(core dumped) ");
      fprintf (stderr, "%s\n", p->command);
    }
  else
    {
      fprintf (stderr, "[%d]\t%d terminated by signal: %d ", j_id, p->pid,
               WTERMSIG (status));
      if (WCOREDUMP (status))
        fprintf (stderr, "(core dumped) ");
      fprintf (stderr, "%s\n", p->command);
    }
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
      if (WIFEXITED (status))
        {
          p->exited = true;
          p->exit_status = WEXITSTATUS (status);
          return;
        }
      if (WIFSTOPPED (status))
        {
          p->stopped = true;
          return;
        }
      if (WIFSIGNALED (status))
        {
          proc_report_signaled (p, status, j->id);
          p->signaled = true;
        }
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
      tcgetattr (STDIN_FILENO, &j->job_term); // save job's termios
      tcsetpgrp (STDIN_FILENO, getpgrp ()); // grub control over terminal back
      tty_restore ();
    }
  if (job_is_stopped (j))
    {
      if (interactive)
        printf ("[%d]\tStopped\t%s\n", j->id, j->command);
      return EXIT_SUCCESS;
    }

  Node *n;
  list_get_head (j->procs, &n);
  process *p = n->data;
  if (p->exited)
    return p->exit_status; // return first process exit status as result for
                           // all job

  if (p->signaled)
    return EXIT_FAILURE;

  errors_fatal ("fsh: unexpected job control error\n");
}

// update_bg_jobs updates state of background jobs in non-blocking way and
// notify about completed
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
    ((process *)curr->data)->stopped = false;
}
