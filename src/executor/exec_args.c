#include "exec_args.h"
#include <stdio.h>
#include "../errors/errors.h"

char **
env_to_kv (HTable *env)
{
  char **res;
  res = calloc (hashtable_size (env) + 1, sizeof (char *));
  if (!res)
    errors_fatal (MEM_ERROR);

  HTableIter hti;
  hashtable_new_iter (env, &hti);

  int i = 0;
  for (Bucket *b; hashtable_iter_next (&hti, &b) != S_ITER_END; ++i)
  {
    char *kv
        = calloc (strlen (b->key) + strlen (b->val) + 2, sizeof (char *));
    if (!kv)
      errors_fatal (MEM_ERROR);
    sprintf (kv, "%s=%s", b->key, b->val);
    res[i] = kv;
  }
  return res;
}

char **args_to_str_arr(char *name, List *args) {
  char **res;
  size_t sz = list_size (args);
  if (sz == 0) return NULL;
  res = calloc (sz + 2, sizeof (char *));
  if (!res)
    errors_fatal (MEM_ERROR);

  Node *head;
  list_get_head(args, &head);

  int i = 0;
  res[i++] = strdup(name);
  while (head) {
    res[i++] = strdup(head->data);
    head = head->next;
  }
  return res;
}

void
free_string_arr (char **arr)
{
  char **start = arr;
  while (*arr)
  {
    free (*arr);
    arr++;
  }
  free (start);
}
