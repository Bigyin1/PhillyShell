#include <stdio.h>
#include "shell.h"

int
main (int argc, char **argv, char **environ)
{
  return start(environ);
}
