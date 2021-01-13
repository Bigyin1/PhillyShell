#include "exec.h"
#include "../builtins/builtins.h"
#include "exec_args.h"
#include "fs.h"
#include "jobs.h"
#include <fcntl.h>
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
cleanup_pipe_fds (int io[2])
{
  if (io)
    {
      close (io[0]);
      close (io[1]);
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

      head = head->next;
    }
}

void
restore_process_state ()
{
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

bool
exec_simple_cmd (sh_executor *e, cmd_node *cn, int inp, int out, bool bg,
                 bool *swap)
{
  char **argv = args_to_str_arr (cn->name, cn->args);

  if (is_non_fork_builtin (cn))
    {
      int status = exec_non_fork_builtin (e, cn, argv);
      free_string_arr (argv);
      if (status == EXIT_SUCCESS && strcmp (cn->name, CMD_FG) == 0)
        {
          *swap = true;
          return true; /* at this point curr_job is swapped to job_spec sent to
                        fg */
        }
      add_new_non_fork_proc_to_job (e->curr_job, status, cn->command);
      return true;
    }

  int pid = fork ();
  if (pid < 0)
    errors_fatal ("fsh: fork failed\n");
  if (pid == 0)
    {
      if (inp == STDIN_FILENO)
        {
          setpgid (0, 0);
          if (!bg)
            tcsetpgrp (0, getpid ());
        } // create new process group for first process in pipeline
      else
        setpgid (0, e->curr_job->pgid);

      restore_process_state ();
      apply_cmd_redirs (cn->redirs);
      apply_cmd_pipe_io (inp, out);

      if (strcmp (cn->name, CMD_ECHO) == 0)
        sh_builtin_echo (argv);
      char *path = find_exe_path (e, cn->name);
      if (!path)
        exit (EXIT_FAILURE);
      if (execve (path, argv, e->kv_env) < 0)
        exit (EXIT_FAILURE);
    }

  if (inp == STDIN_FILENO)
    {
      setpgid (pid, pid);
      if (!bg)
        tcsetpgrp (0, pid);
      e->curr_job->pgid = pid;
    }

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
  bool is_curr_job_swapped = false;

  e->curr_job = new_job (++e->last_jb_id, pn->command, bg);

  e->bg_fg_enabled = true;
  if (list_size (pn->procs) > 1)
    e->bg_fg_enabled = false; /* disabling bg an fg utils in case of starting
                               pipeline of > 1 procs */

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

      bool non_fork
          = exec_simple_cmd (e, cn, inp, out, bg, &is_curr_job_swapped);

      if (out != STDOUT_FILENO)
        close (out);
      if (inp != STDIN_FILENO)
        close (inp);

      inp = p[0];
      if (non_fork)
        {
          close (inp);
          inp = STDIN_FILENO;
        }
    }

#ifdef DEBUG
  printf ("%d\n", e->curr_job->pgid);
#endif

  if (is_curr_job_swapped)
    return get_job_exit_code (e->curr_job);
  if (bg)
    printf ("[%d] %d\n", e->curr_job->id, e->curr_job->pgid);
  add_new_job_to_list (e->active_jobs, e->curr_job);
  return bg ? EXIT_SUCCESS : get_job_exit_code (e->curr_job);
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

int
exec_if_list_or_pipe (sh_executor *e, sh_ast_node root, bool bg)
{
  node_type t = get_node_type (root);
  int status;
  if (t == NODE_IF)
    status = exec_if_list (e, root);
  if (t == NODE_PIPE)
    status = exec_pipeline (e, root, bg);

  return status;
}

int
exec_list (sh_executor *e, list_node *ln)
{
  int status;
  bool bg = false;

  if (ln->token->type == SH_T_AMP) // if &
    bg = true;

  status = exec_if_list_or_pipe (e, ln->cont, bg);
  if (ln->next)
    status = exec_list (e, ln->next) && status;

  return status;
}

int
exec_ast (sh_executor *e, sh_ast_node root)
{
  node_type t = get_node_type (root);
  int status;

  if (t == NODE_SEP)
    status = exec_list (e, root);
  if (t == NODE_IF)
    status = exec_if_list (e, root);
  if (t == NODE_PIPE)
    status = exec_pipeline (e, root, false);

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

sh_ecode
execute_cmd (sh_executor *e, char *cmd)
{
  sh_tokenizer t = { 0 };
  sh_parser p = { .cmd = cmd };
  sh_ecode err;

  update_bg_jobs (e->active_jobs);
  e->last_jb_id = get_last_job_id (e->active_jobs);

  e->kv_env = env_to_kv (e->env);
  if (tokenize (&t, cmd) != SH_OK)
    return SH_ERR;
  if (parse_tokens (&p, &t) != SH_OK)
    return SH_ERR;

  debug_info (&t, &p);

  err = exec_list (e, p.root_node);

  parser_free (&p);
  free_string_arr (e->kv_env);

  return err;
}
