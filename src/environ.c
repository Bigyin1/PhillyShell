#include "environ.h"
#include <stdio.h>

sh_err
parse_environ (Shell *sh, char **environ)
{
  char *key, *val;
  for (; *environ; environ++)
    {
      key = strtok_r (*environ, "=", &val);
      key = strdup (key);
      if (!key)
        return SH_FATAL;
      val = strdup (val);
      if (!val)
        return SH_FATAL;

      if (hashtable_set (sh->env, key, val) != S_OK)
        return SH_FATAL;
    }
  return SH_OK;
}

sh_err
parse_path (Shell *sh)
{
  char *path, *path_elem;

  path = (char *)hashtable_get (sh->env, PATH);

  while ((path_elem = strtok (path, ":")))
    {
      path = NULL;
      if (slice_append (sh->path, path_elem) != S_OK)
        return SH_FATAL;
    }

  return SH_OK;
}
