#include "exec_cmd.h"
#include "cmd_parser.h"
#include "cmd_tokenizer.h"

extern char error_buf[512];

sh_err
exec_cmd (Shell *sh)
{
  sh_err err = { 0 };
  sh_tokenizer t = { 0 };
  sh_parser p = { 0 };
  cmd_list_node *root;

  err = tokenize (&t, sh->cmd_buf);
  if (err.code != SH_OK)
    return err;
  p.t = &t;
  parse(&p, &root);

  list_free (t.tokens, token_free);
  return err;
}
