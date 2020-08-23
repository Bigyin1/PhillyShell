#include "exec.h"
#include "../errors/errors.h"
#include "exec_args.h"
#include "fs.h"
#include <errno.h>
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
      close(inp[0]);
    }
  if (out)
    {
      close (out[0]);
      dup2 (out[1], STDOUT_FILENO);
      close(out[1]);
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
    return ; // no redirs;

  while (head)
    {
      redir = head->data;

      if (redir->to_file)
        {
          int flags = O_WRONLY| O_CREAT;
          flags = redir->append ? (flags | O_APPEND) : flags;
          int o_fd = open (redir->to_file, flags,
                           S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
          if (o_fd == -1)
              exit (EXIT_FAILURE);
          if (dup2 (o_fd, redir->from_fd) == -1)
              exit (EXIT_FAILURE);
        }
      if (redir->from_file) {
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

int
exec_simple_cmd (sh_executor *e, cmd_node *cn, int inp[2], int out[2])
{
  char *path = find_exe_path (e, cn->name);
  if (!path)
    return 0;
  char **args = args_to_str_arr (cn->name, cn->args);
  int pid = fork ();
  if (pid < 0)
    errors_fatal ("fsh: fork failed\n");
  if (pid == 0)
    {
      apply_cmd_redirs(cn->redirs);
      apply_cmd_pipe_io (inp, out);
      if (execve (path, args, e->kv_env) < 0)
        errors_fatal ("fsh: exec failed\n");
    }
  free_string_arr (args);
  free (path);
  return pid;
}

// returns number of processes that was forked by this and all subsequent
// pipes. Returns 0 on error
int
exec_pipeline (sh_executor *e, bin_op_node *pn, int inp[2])
{
  int thin_red_line[2];

  if (pipe (thin_red_line) == -1)
    errors_fatal ("fsh: pipe failed\n");
  node_type l = get_node_type (pn->left);
  if (l != NODE_CMD)
    errors_fatal ("fsh: unexpected parser error\n");
  node_type r = get_node_type (pn->right);

  int res = exec_simple_cmd (e, pn->left, inp, thin_red_line);
  cleanup_pipe_fds (inp);
  if (res == 0)
    {
      cleanup_pipe_fds (thin_red_line);
      return 0;
    }

  if (r == NODE_PIPE)
    {
      int procs = exec_pipeline (e, pn->right, thin_red_line);
      if (procs == 0)
        return 0;
      return procs + 1;
    }

  int last_pid = exec_simple_cmd (e, pn->right, thin_red_line, NULL);
  cleanup_pipe_fds (thin_red_line);
  if (last_pid == 0)
    return 0;

  int wstatus;
  if (waitpid (last_pid, &wstatus, 0) == -1)
    errors_fatal ("fsh: waitpid failed\n");
  e->last_retcode = WEXITSTATUS (wstatus);
  return 1;
}

sh_ecode
exec_ast (sh_executor *e, sh_ast_node root)
{
  node_type t = get_node_type (root);
  if (t == NODE_CMD)
    {
      int pid = exec_simple_cmd (e, root, NULL, NULL);
      if (!pid)
        return SH_ERR;

      int wstatus;
      if (waitpid (pid, &wstatus, 0) == -1)
        errors_fatal ("fsh: waitpid failed\n");
      e->last_retcode = WEXITSTATUS (wstatus);
    }
  if (t == NODE_PIPE)
    {
      int to_wait = exec_pipeline (e, root, NULL);
      for (int i = 0; i < to_wait; ++i)
        wait (NULL);
    }

  return SH_OK;
}

void
exec_init (sh_executor *e, Slice *path, HTable *vars)
{
  e->kv_env = env_to_kv (vars);
  e->path_var = path;
}

void
exec_free (sh_executor *e)
{
  free_string_arr (e->kv_env);
}

sh_ecode
sh_exec (Shell *sh)
{
  sh_tokenizer t = { 0 };
  sh_parser p = { 0 };
  sh_executor e = { 0 };

  if (tokenize (&t, sh->cmd_buf) != SH_OK)
    return SH_ERR;

  p.tok = &t;
  p.vars = sh->env;
  if (parse_tokens (&p) != SH_OK)
    return SH_ERR;

  exec_init (&e, sh->path, sh->env);
//  ast_dump (p.root_node, stdout); // for debug
//  fflush (stdout);
  sh_ecode err = exec_ast (&e, p.root_node);

  parser_free (&p);
  exec_free (&e);

  return err;
}
