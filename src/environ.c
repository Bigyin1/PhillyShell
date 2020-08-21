#include "environ.h"
#include "errors/errors.h"
#include <stdio.h>

sh_ecode
parse_environ (Shell *sh, char **environ)
{
  char *key, *val;
  for (; *environ; environ++)
    {
      key = strtok_r (*environ, "=", &val);
      key = strdup (key);
      if (!key)
        errors_fatal (MEM_ERROR);
      val = strdup (val);
      if (!val)
        errors_fatal (MEM_ERROR);

      if (hashtable_set (sh->env, key, val) != S_OK)
        errors_fatal (MEM_ERROR);
    }
  return SH_OK;
}

sh_ecode
parse_path (Shell *sh)
{
  char *path, *path_elem;

  path = (char *)hashtable_get (sh->env, PATH);

  while ((path_elem = strtok (path, ":")))
    {
      path = NULL;
      if (slice_append (sh->path, path_elem) != S_OK)
        errors_fatal (MEM_ERROR);
    }

  return SH_OK;
}
