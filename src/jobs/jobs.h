#ifndef PHILLYSHELL_JOBS_H
#define PHILLYSHELL_JOBS_H

#include <structs/list.h>
#include <termios.h>

/* A process is a single process.  */
typedef struct process
{
  char *command;
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
  uint id;

  struct termios job_term;
  bool is_background;
} job;


bool job_delete_func (void *p);
int get_job_exit_code (job *j, bool is_interactive);
bool job_is_stopped (job *j);
bool job_is_completed (job *j);
bool job_any_running_procs (job *j);
void update_bg_jobs (List *jl);
int get_last_job_id (List *jl);
void job_continue (job *j, bool in_foreground);
void job_set_to_foreground (job *j);

job *new_job (uint id, char *cmd, bool bg, bool interactive);
void add_new_proc_to_job (job *j, pid_t pid, char *cmd);
void add_new_non_fork_proc_to_job (job *j, int status, char *cmd);
void add_new_job_to_list (List *jl, job *j);

#endif // PHILLYSHELL_JOBS_H
