#include "environ.h"
#include "../errors/errors.h"
#include <stdio.h>

sh_ecode
parse_environ (Shell *sh, char **env)
{
  size_t len;
  for (; *env; env++)
    {
      char *var = *env;
      len = strcspn (*env, "=");
      var[len] = '\0';

      if (hashtable_set (sh->e.env, var, var + len + 1) != S_OK)
        errors_fatal (MEM_ERROR);
    }
  return SH_OK;
}

sh_ecode
parse_path_var (Shell *sh)
{
  char *path, *path_elem;

  path = (char *)hashtable_get (sh->e.env, PATH);

  while ((path_elem = strtok (path, ":")))
    {
      path = NULL;
      if (slice_append (sh->e.path_var, path_elem) != S_OK)
        errors_fatal (MEM_ERROR);
    }

  return SH_OK;
}
