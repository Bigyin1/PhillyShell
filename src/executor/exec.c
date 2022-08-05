#include "exec.h"
#include "../builtins/builtins.h"
#include "exec_args.h"
#include "fs.h"
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

void
apply_cmd_pipe_io (int inp, int out)
{
  if (inp != STDIN_FILENO)
    {
      dup2 (inp, STDIN_FILENO);
      close (inp);
    }
  if (out != STDOUT_FILENO)
    {
      dup2 (out, STDOUT_FILENO);
      close (out);
    }
}

void
apply_cmd_redirs (List *redirs)
{
  struct s_redir *redir;
  Node *head;
  if (list_get_head (redirs, &head) != S_OK)
    return; // no redirs;

  while (head)
    {
      redir = head->data;

      if (redir->to_file)
        {
          int flags = O_WRONLY | O_CREAT;
          flags = redir->append ? (flags | O_APPEND) : flags;
          int o_fd = open (redir->to_file, flags,
                           S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
          if (o_fd == -1)
            exit (EXIT_FAILURE);
          if (dup2 (o_fd, redir->from_fd) == -1)
            exit (EXIT_FAILURE);
        }
      if (redir->from_file)
        {
          int flags = O_RDONLY;
          int f_fd = open (redir->from_file, flags);
          if (f_fd == -1)
            exit (EXIT_FAILURE);
          if (dup2 (f_fd, redir->to_fd) == -1)
            exit (EXIT_FAILURE);
        }

      if (!redir->from_file && !redir->to_file)
        {
          if (dup2 (redir->from_fd, redir->to_fd) == -1)
            exit (EXIT_FAILURE);
        }

      head = head->next;
    }
}

void
restore_process_state (sh_executor *e)
{
  if (!e->is_interactive)
    return;
  signal (SIGINT, SIG_DFL);
  signal (SIGQUIT, SIG_DFL);
  signal (SIGTSTP, SIG_DFL);
  signal (SIGTTIN, SIG_DFL);
  signal (SIGTTOU, SIG_DFL);
}

bool
is_non_fork_builtin (cmd_node *cn)
{
  if (strcmp (cn->name, CMD_EXIT) == 0 || strcmp (cn->name, CMD_CD) == 0
      || strcmp (cn->name, CMD_JOBS) == 0 || strcmp (cn->name, CMD_BG) == 0
      || strcmp (cn->name, CMD_FG) == 0)
    return true;
  return false;
}

int
exec_non_fork_builtin (sh_executor *e, cmd_node *cn, char **argv)
{
  if (strcmp (cn->name, CMD_EXIT) == 0)
    sh_builtin_exit ();
  if (strcmp (cn->name, CMD_CD) == 0)
    return sh_builtin_cd (argv, e->env);
  if (strcmp (cn->name, CMD_JOBS) == 0)
    {
      sh_builtin_jobs (e);
      return EXIT_SUCCESS;
    }
  if (strcmp (cn->name, CMD_BG) == 0)
    return sh_builtin_bg (e, argv);
  if (strcmp (cn->name, CMD_FG) == 0)
    return sh_builtin_fg (e, argv);
}

void
add_process_to_jobs_pgroup (job *curr_job, int pid, bool bg)
{
  if (curr_job->pgid == -1) // if no dedicated pgroup
    {
      curr_job->pgid = pid;
      setpgid (pid, curr_job->pgid);
      if (!bg)
        tcsetpgrp (STDIN_FILENO, pid);
    }
  else
    setpgid (pid, curr_job->pgid);
}

bool
exec_simple_cmd (sh_executor *e, cmd_node *cn, int inp, int out, bool bg)
{
  char **argv = args_to_str_arr (cn->name, cn->args);

  if (is_non_fork_builtin (cn))
    {
      int status = exec_non_fork_builtin (e, cn, argv);
      free_string_arr (argv);
      if (status == EXIT_SUCCESS && strcmp (cn->name, CMD_FG) == 0)
        return true; /* at this point curr_job is swapped to job_spec sent to
                      fg */
      add_new_non_fork_proc_to_job (e->curr_job, status, cn->command);
      return true;
    }

  int pid = fork ();
  if (pid < 0)
    errors_fatal ("fsh: fork failed\n");
  if (pid == 0)
    {
      if (e->is_interactive)
        add_process_to_jobs_pgroup (e->curr_job, getpid (), bg);

      restore_process_state (e);
      apply_cmd_redirs (cn->redirs);
      apply_cmd_pipe_io (inp, out);

      if (strcmp (cn->name, CMD_ECHO) == 0)
        sh_builtin_echo (argv);
      char *path = find_exe_path (e, cn->name);
      if (!path)
        exit (EXIT_FAILURE);
      if (execve (path, argv, env_to_kv (e->env)) < 0)
        exit (EXIT_FAILURE);
    }

  if (e->is_interactive)
    add_process_to_jobs_pgroup (e->curr_job, pid,
                                bg); // repeated, cause of race condition

  add_new_proc_to_job (e->curr_job, pid, cn->command);
  free_string_arr (argv);
  return false;
}

int
exec_pipeline (sh_executor *e, pipeline_node *pn, bool bg)
{
  int inp;
  int out;
  int p[2];

  e->curr_job = new_job (++e->last_jb_id, pn->command, bg, e->is_interactive);

  e->bg_fg_enabled = true;
  if (list_size (pn->procs) > 1 || bg || !e->is_interactive)
    e->bg_fg_enabled
        = false; /* disabling bg an fg utils in case of starting
                  pipeline of > 1 procs or in background or non interactive*/

  Node *n;
  if (list_get_head (pn->procs, &n) != S_OK)
    errors_fatal ("fsh: unexpected parser error");

  inp = STDIN_FILENO;
  for (; n; n = n->next)
    {
      cmd_node *cn = n->data;

      if (!n->next)
        out = STDOUT_FILENO;
      else if (pipe (p) == 0)
        out = p[1];
      else
        errors_fatal ("fsh: pipe failed\n");

      bool non_fork = exec_simple_cmd (e, cn, inp, out, bg);

      if (out != STDOUT_FILENO)
        close (out);
      if (inp != STDIN_FILENO)
        close (inp);

      inp = p[0];
      if (non_fork && n->next)
        {
          close (inp);
          inp = STDIN_FILENO;
        }
    }

  if (bg && e->is_interactive)
    printf ("[%d] %d\n", e->curr_job->id, e->curr_job->pgid); // notify user
  if (e->is_interactive)
    add_new_job_to_list (
        e->active_jobs,
        e->curr_job); // we have to track all jobs in interactive mode
  if (bg)
    return EXIT_SUCCESS;

  int ecode = get_job_exit_code (e->curr_job, e->is_interactive);
  if (!e->is_interactive)
    job_delete_func (e->curr_job);
  return ecode;
}

int
exec_if_list (sh_executor *e, bin_op_node *bn)
{
  int status_left;
  if (get_node_type (bn->left) == NODE_PIPE)
    status_left = exec_pipeline (e, bn->left, false);
  else
    status_left = exec_if_list (e, bn->left);

  if (bn->token->type == SH_T_AND_IF && status_left != 0)
    return status_left;
  if (bn->token->type == SH_T_OR_IF && status_left == 0)
    return status_left;

  int status_right;
  if (get_node_type (bn->right) == NODE_PIPE)
    status_right = exec_pipeline (e, bn->right, false);
  else
    status_right = exec_if_list (e, bn->right);

  if (bn->token->type == SH_T_AND_IF)
    return status_left && status_right;
  if (bn->token->type == SH_T_OR_IF)
    return status_left || status_right;

  errors_fatal ("fsh: unexpected error\n");
}

void
if_list_subshell (sh_executor *e, list_node *ln)
{
  job *subshell_job
      = new_job (++e->last_jb_id, ln->command, true, e->is_interactive);

  int pid = fork ();
  if (pid < 0)
    errors_fatal ("fsh: fork failed\n");
  if (pid == 0)
    {
      setpgid (0, 0); // subshell's jobs will be in this new process group
      restore_process_state (e);
      e->is_interactive = false;
      exit (exec_if_list (e, (bin_op_node *)ln->cont));
    }

  if (!e->is_interactive)
    return;
  setpgid (pid, pid);
  subshell_job->pgid = pid;
  add_new_proc_to_job (subshell_job, pid, NULL);
  add_new_job_to_list (e->active_jobs, subshell_job);
  printf ("[%d] %d\n", subshell_job->id, subshell_job->pgid);
}

int
exec_if_list_or_pipe (sh_executor *e, list_node *ln, bool bg)
{
  node_type t = get_node_type (ln->cont);

  if (!bg)
    free (ln->command);
  if (t == NODE_IF)
    {
      if (!bg)
        return exec_if_list (e, (bin_op_node *)ln->cont);

      if_list_subshell (e, ln);
      return EXIT_SUCCESS;
    }
  if (t == NODE_PIPE)
    return exec_pipeline (e, (pipeline_node *)ln->cont, bg);
}

int
exec_list (sh_executor *e, list_node *ln)
{
  int status;
  bool bg = false;

  if (ln->token->type == SH_T_AMP) // if &
    bg = true;

  status = exec_if_list_or_pipe (e, ln, bg);
  if (ln->next)
    status = exec_list (e, ln->next) && status;

  return status;
}

void
debug_info (sh_tokenizer *t, sh_parser *p)
{
#ifdef DEBUG
  tokenizer_dump (t, stdout);
  ast_dump (p->root_node, stdout);
  fprintf (stdout, "\n");
#endif
}

void
executor_free (sh_executor *e)
{
  hashtable_free (e->env);
  slice_free (e->path_var, NULL);
  list_free (e->active_jobs, job_delete_force_func);
}

sh_ecode
execute_cmd (sh_executor *e, char *cmd)
{
  sh_tokenizer t = { 0 };
  sh_parser p = { .cmd = cmd };
  sh_ecode err;

  if (e->is_interactive)
    update_bg_jobs (e->active_jobs);
  list_filter_mod (e->active_jobs, job_delete_func); // deleting completed jobs
  e->last_jb_id = get_last_job_id (e->active_jobs);

  if (tokenize (&t, cmd) != SH_OK)
    return SH_ERR;
  if (parse_tokens (&p, &t) != SH_OK)
    return SH_ERR;

  debug_info (&t, &p);

  err = exec_list (e, p.root_node);

  parser_free (&p);
  return err;
}
