#include "../shell.h"
#include <stdlib.h>

extern Shell *glob_shell;

void
sh_builtin_exit ()
{
  shell_free (glob_shell);
  exit (EXIT_SUCCESS);
}