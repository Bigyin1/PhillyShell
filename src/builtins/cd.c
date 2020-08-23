#include "builtins.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

int
sh_builtin_cd (char **argv, HTable *env)
{
  argv++;
  char *home = hashtable_get (env, "HOME");
  char *path = *argv ? argv[0] : home;

  if (chdir (path) == -1)
    {
      printf ("cd: %s\n", strerror (errno));
      return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}