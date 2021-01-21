#include "shell.h"
#include "environ/environ.h"
#include "errors/errors.h"
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

sh_ecode
config_init (t_config *cfg)
{
  cfg->prompt = "$> ";
  return SH_OK;
}

void
shell_free (Shell *sh)
{
  free (sh->cfg);
  hashtable_free (sh->e.env);
  slice_free (sh->e.path_var, NULL);
}

sh_ecode
shell_init (Shell *sh)
{
  signal (SIGINT, SIG_IGN);
  signal (SIGQUIT, SIG_IGN);
  signal (SIGTSTP, SIG_IGN);
  signal (SIGTTIN, SIG_IGN);
  signal (SIGTTOU, SIG_IGN);

  sh->cfg = calloc (1, sizeof (t_config));
  if (!sh->cfg)
    errors_fatal (MEM_ERROR);

  t_ret rt = new_list (&sh->e.active_jobs);
  if (rt != S_OK)
    errors_fatal (MEM_ERROR);
  rt = hashtable_new (&sh->e.env, string_hash,
                      (int (*) (void *, void *))strcmp, NULL, NULL);
  if (rt != S_OK)
    errors_fatal (MEM_ERROR);
  rt = slice_new (&sh->e.path_var, 0, 32);
  if (rt != S_OK)
    errors_fatal (MEM_ERROR);

  if (config_init (sh->cfg) != SH_OK)
    return SH_ERR;

  sh->e.last_jb_id = 0;
  sh->e.in_subshell = false;
  return SH_OK;
}

void
shell_print_prompt (Shell *sh)
{
  printf ("%s", sh->cfg->prompt);
  fflush (stdout);
}

sh_ecode
main_loop (Shell *sh)
{
  while (1)
    {
      shell_print_prompt (sh);
      if (fgets (sh->cmd_buf, MAX_INPUT, stdin) == NULL)
        {
          perror ("fsh:");
          return SH_ERR;
        }
      sh->cmd_buf[strlen(sh->cmd_buf)-1] = 0;
      execute_cmd (&sh->e, sh->cmd_buf);
    }
}

int
start (int argc, char **argv, char **environ)
{
  Shell sh;

  if (shell_init (&sh) != SH_OK)
    {
      shell_free (&sh);
      return EXIT_FAILURE;
    }

  // preparing environment and path var
  parse_environ (&sh, environ);
  parse_path_var (&sh);

  if (argc > 2 && strcmp (argv[1], "-c") == 0)
    {
      argv += 2;
      while (*argv)
        {
          strcat (sh.cmd_buf, *argv++);
          strcat (sh.cmd_buf, " ");
        }
      execute_cmd (&sh.e, sh.cmd_buf);
      shell_free (&sh);
      return EXIT_SUCCESS;
    }
  sh_ecode err = main_loop (&sh);
  shell_free (&sh);
  if (err == SH_OK)
    return EXIT_SUCCESS;
  return EXIT_FAILURE;
}
