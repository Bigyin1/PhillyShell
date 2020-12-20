#ifndef PHILLYSHELL_JOB_CONTROL_H
#define PHILLYSHELL_JOB_CONTROL_H

#include <structs/list.h>

/* A process is a single process.  */
typedef struct process
{
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
} job;

void wait_for_job (job *j);

bool job_delete_func (void *p);

#endif // PHILLYSHELL_JOB_CONTROL_H
