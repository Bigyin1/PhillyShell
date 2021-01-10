#ifndef PHILLYSHELL_JOBS_H
#define PHILLYSHELL_JOBS_H

#include <structs/list.h>

/* A process is a single process.  */
typedef struct process
{
  char *name;
  pid_t pid;      /* process ID */
  int status;     /* reported status value */
  bool completed; /* true if process has completed */
  bool stopped;   /* true if process has stopped */
} process;

/* A job is a pipeline of processes.  */
typedef struct job
{
  char *command; /* command line, used for messages */
  List *procs;
  pid_t pgid; /* process group ID */
  bool background;
} job;

void wait_for_job (job *j);
bool job_delete_func (void *p);
int get_job_exit_code (job *j);
bool job_is_stopped (job *j);

job *new_job ();
void add_new_proc_to_job (job *j, pid_t pid);

#endif // PHILLYSHELL_JOBS_H
