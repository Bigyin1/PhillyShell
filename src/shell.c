#include "shell.h"
#include "environ.h"
#include "errors/errors.h"
#include "executor/exec_cmd.h"
#include <stdlib.h>

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
  hashtable_free (sh->env);
  slice_free (sh->path, NULL);
}

sh_ecode
shell_init (Shell *sh)
{
  sh->cfg = calloc (1, sizeof (t_config));
  if (!sh->cfg)
    errors_fatal (MEM_ERROR);
  t_ret rt = hashtable_new (&sh->env, string_hash,
                            (int (*) (void *, void *))strcmp, free, free);
  if (rt != S_OK)
    errors_fatal (MEM_ERROR);
  rt = slice_new (&sh->path, 0, 32);
  if (rt != S_OK)
    errors_fatal (MEM_ERROR);

  if (config_init (sh->cfg) != SH_OK)
    return SH_ERR;
  return SH_OK;
}

void
shell_print_prompt (Shell *sh)
{
  fprintf (sh->out, "%s", sh->cfg->prompt);
  fflush (sh->out);
}

sh_ecode
main_loop (Shell *sh)
{
  while (sh->running)
    {
      shell_print_prompt (sh);
      if (fgets (sh->cmd_buf, MAX_INPUT, sh->inp) == NULL)
        return SH_ERR;
      exec_cmd (sh);
    }

  return SH_OK;
}

int
start (char **environ)
{
  Shell sh = { 0 };
  sh.out = stdout;
  sh.inp = stdin;

  if (shell_init (&sh) != SH_OK)
    {
      shell_free (&sh);
      return EXIT_FAILURE;
    }

  // preparing environment and path var
  parse_environ (&sh, environ);
  parse_path (&sh);

  sh.running = true;
  sh_ecode err = main_loop (&sh);
  shell_free (&sh);

  if (err == SH_OK)
    return EXIT_SUCCESS;
  return EXIT_FAILURE;
}
