#include "shell.h"
#include <stdio.h>
#include <stdlib.h>

int
fill_config (t_config *cfg)
{
  cfg->prompt = "$> ";
  return EXIT_SUCCESS;
}


int
start (char **environ)
{
  t_config cfg;
  fill_config (&cfg);
  Shell sh;
  sh.cfg = &cfg;

  while (*environ)
    {
      printf ("%s\n", *environ);
      environ++;
    }
  return EXIT_SUCCESS;
}
