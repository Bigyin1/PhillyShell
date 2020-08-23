#include <stdio.h>
#include <stdlib.h>

void
sh_builtin_echo (char **argv)
{
  argv++;
  while (*argv)
    {
      printf ("%s", *argv);
      if (*(argv + 1))
        printf (" ");
      argv++;
    }
  printf("\n");
  exit (EXIT_SUCCESS);
}