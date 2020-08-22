#include "exec.h"
#include "../errors/errors.h"
#include "exec_args.h"
#include "fs.h"
#include <sys/wait.h>


sh_ecode
exec_ast (sh_executor *e)
{
  node_type t = get_node_type (e->root_node);
  if (t == NODE_CMD)
    {
      cmd_node *cn = (cmd_node *)e->root_node;
      char *path = find_exe_path (e, cn->name);
      if (!path)
        {
          return SH_ERR;
        }
      char **args = args_to_str_arr (cn->name, cn->args);
      if (fork () == 0)
        {
          if (execve (path, args, e->kv_env) < 0)
            errors_fatal ("fsh: exec failed");
        }
      wait (NULL);
      free_string_arr (args);
      free(path);
    }

  return SH_OK;
}

void
exec_init (sh_executor *e, Slice *path, HTable *vars, sh_ast_node root)
{
  e->kv_env = env_to_kv (vars);
  e->root_node = root;
  e->path_var = path;
}

void exec_free(sh_executor *e) {
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

  exec_init (&e, sh->path, sh->env, p.root_node);
  sh_ecode err = exec_ast (&e);

  parser_free(&p);
  exec_free(&e);

  return err;
}
