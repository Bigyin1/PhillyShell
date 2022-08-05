#include "shell.h"
#include "environ/environ.h"
#include "errors/errors.h"
#include "executor/exec.h"
#include "jobs/jobs.h"
#include "shell.h"
#include "tty/tty.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

Shell *glob_shell;

void
shell_free (Shell *sh)
{
  executor_free (&sh->e);
}

sh_ecode
shell_init (Shell *sh)
{
  sh->e.is_interactive = isatty (STDIN_FILENO);
  if (sh->e.is_interactive)
    {
      signal (SIGINT, SIG_IGN);
      signal (SIGQUIT, SIG_IGN);
      signal (SIGTSTP, SIG_IGN);
      signal (SIGTTIN, SIG_IGN);
      signal (SIGTTOU, SIG_IGN);

      tty_save ();
    }

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

  sh->e.last_jb_id = 0;
  return SH_OK;
}

_Noreturn sh_ecode
main_loop (Shell *sh)
{
  while (1)
    {
      tty_setraw (hashtable_get (sh->e.env, "TERM"));
      String *s = tty_readline ();
      tty_restore ();
      execute_cmd (&sh->e, string_to_c_str (s));
      string_free (s);
    }
}

int
start (int argc, char **argv, char **environ)
{
  Shell sh = { 0 };

  glob_shell = &sh;
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

int
main (int argc, char **argv, char **environ)
{
  return start (argc, argv, environ);
}
