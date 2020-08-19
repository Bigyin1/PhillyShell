#include "shell.h"
#include "environ.h"
#include "exec_cmd.h"
#include <stdlib.h>

char error_buf[512];

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
    return SH_FATAL;
  t_ret rt = hashtable_new (&sh->env, string_hash, strcmp, free, free);
  if (rt != S_OK)
    return SH_FATAL;
  rt = slice_new (&sh->path, 0, 32);
  if (rt != S_OK)
    return SH_FATAL;

  if (config_init (sh->cfg) != SH_OK)
    {
      shell_free (sh);
      return SH_FATAL;
    }

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
        return SH_FATAL;
      sh_err err = exec_cmd(sh);
      if (err.code == SH_FATAL) return SH_FATAL;
      if (err.code == SH_MODERATE) {
          fprintf(stderr, "%s", error_buf);
        }
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
