#include <stdio.h>
#include <stdlib.h>

void
errors_fatal (char *err) {
  fprintf(stderr, "%s", err);
  exit(EXIT_FAILURE);
}

