#include "fs.h"
#include "../errors/errors.h"
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>

bool
is_executable (char *path)
{
  return !access (path, X_OK);
}

char *
find_exe_path (sh_executor *e, char *name)
{
  char path[PATH_MAX];
  if (strchr (name, '/'))
    {
      if (is_executable (name))
        {
          return name;
        }
      printf ("fsh: %s", strerror (errno));
      return NULL;
    }

  for (int i = 0; i < slice_len (e->path_var); ++i)
    {
      char *path_elem;
      if (slice_get (e->path_var, i, (void **)&path_elem) != S_OK)
        errors_fatal ("fsh: unexpected");
      memset (path, 0, PATH_MAX);
      strcat (path, path_elem);
      if (path[strlen (path) - 1] != '/')
        path[strlen (path)] = '/';
      strcat (path, name);
      if (is_executable (path))
        {
          char *res = strdup (path);
          return res;
        }
      if (errno == EACCES)
        printf ("fsh: %s\n", strerror (errno));
    }
  printf ("fsh: %s\n", strerror (errno));
  return NULL;
}