#include "exec.h"
#include "../builtins/builtins.h"
#include "exec_args.h"
#include "fs.h"
#include "job_control.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

void
apply_cmd_pipe_io (int *inp, int *out)
{
  if (inp)
    {
      close (inp[1]);
      dup2 (inp[0], STDIN_FILENO);
      close (inp[0]);
    }
  if (out)
    {
      close (out[0]);
      dup2 (out[1], STDOUT_FILENO);
      close (out[1]);
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

void
exec_simple_cmd (sh_executor *e, cmd_node *cn, int inp[2], int out[2])
{
  char **argv = args_to_str_arr (cn->name, cn->args);

  if (strcmp (cn->name, CMD_EXIT) == 0)
    sh_builtin_exit ();
  if (strcmp (cn->name, CMD_CD) == 0)
    {
      sh_builtin_cd (argv, e->env);
      return;
    }
  if (strcmp (cn->name, CMD_JOBS) == 0)
    {
      sh_builtin_jobs (e);
      return;
    }

  int pid = fork ();
  if (pid < 0)
    errors_fatal ("fsh: fork failed\n");
  if (pid == 0)
    {
      if (inp == NULL)
        {
          setpgid (0, 0);
          tcsetpgrp (0, getpid ());
        } // create new process group for first process in pipeline
      else
        setpgid (0, e->curr_job->pgid);

      restore_process_state ();
      apply_cmd_redirs (cn->redirs);
      apply_cmd_pipe_io (inp, out);

      if (strcmp (cn->name, CMD_ECHO) == 0)
        sh_builtin_echo (argv);
      // printf("tcgetpgrp: %d; curr pgid: %d\n", tcgetpgrp(1), getpgid(0));
      char *path = find_exe_path (e, cn->name);
      if (!path)
        exit (EXIT_FAILURE);
      if (execve (path, argv, e->kv_env) < 0)
        exit (EXIT_FAILURE);
    }

  process *curr_proc = calloc (1, sizeof (process));
  if (!curr_proc)
    exit (EXIT_FAILURE);
  curr_proc->pid = pid;

  if (inp == NULL)
    {
      setpgid (pid, pid);
      tcsetpgrp (0, pid);
      e->curr_job->pgid = pid;
    }

  list_push_back (e->curr_job->procs, curr_proc);
  free_string_arr (argv);
}

void
exec_pipeline (sh_executor *e, bin_op_node *pn, int inp[2])
{
  int thin_red_line[2];

  if (pipe (thin_red_line) == -1)
    errors_fatal ("fsh: pipe failed\n");
  node_type l = get_node_type (pn->left);
  if (l != NODE_CMD)
    errors_fatal ("fsh: unexpected parser error\n");
  node_type r = get_node_type (pn->right);

  exec_simple_cmd (e, pn->left, inp, thin_red_line);
  cleanup_pipe_fds (inp);

  if (r == NODE_PIPE)
    exec_pipeline (e, pn->right, thin_red_line);
  else
    {
      exec_simple_cmd (e, pn->right, thin_red_line, NULL);
      cleanup_pipe_fds (thin_red_line);
    }
}

sh_ecode
exec_ast (sh_executor *e, sh_ast_node root)
{
  node_type t = get_node_type (root);
  if (t == NODE_CMD)
    exec_simple_cmd (e, root, NULL, NULL);
  if (t == NODE_PIPE)
    exec_pipeline (e, root, NULL);
  wait_for_job (e->curr_job);
  return SH_OK;
}

sh_ecode
execute_cmd (sh_executor *e, char *cmd)
{
  sh_tokenizer t;
  sh_parser p;
  sh_ecode err;

  e->kv_env = env_to_kv (e->env);
  if (tokenize (&t, cmd) != SH_OK)
    return SH_ERR;
#ifdef DEBUG
  tokenizer_dump(&t, stdout);
#endif

  if (parse_tokens (&p, &t) != SH_OK)
    return SH_ERR;
#ifdef DEBUG
  ast_dump(p.root_node, stdout);
  printf("\n");
#endif

  job *curr_job = calloc (1, sizeof (struct job));
  if (!curr_job)
    return SH_ERR;

  if (new_list (&curr_job->procs) != S_OK)
    errors_fatal (MEM_ERROR);

  e->curr_job = curr_job;
  list_push_back (e->active_jobs, curr_job);
  e->curr_job->command = cmd;

  err = exec_ast (e, p.root_node);

  // printf("job [%d] (%s) done\n", e->curr_job->pgid, e->curr_job->command);
  list_filter_mod (e->active_jobs, job_delete_func);
  parser_free (&p);
  free_string_arr (e->kv_env);

  e->curr_job = NULL;
  return err;
}
