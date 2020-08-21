#include <stdio.h>
#include <stdlib.h>

void
errors_fatal (char *err) {
  printf("%s", err);
  exit(EXIT_FAILURE);
}

