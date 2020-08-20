#include "exec_cmd.h"
#include "cmd_tokenizer.h"
#include "parser/parser.h"
#include "parser/utils.h"

extern char error_buf[512];

sh_err
exec_cmd (Shell *sh)
{
  sh_err err;
  sh_tokenizer t = { 0 };
  sh_parser p = { 0 };

  err = tokenize (&t, sh->cmd_buf);
  if (err.code != SH_OK)
    {
      list_free (t.tokens, token_free);
      return err;
    }

  p.tok = &t;
  p.vars = sh->env;
  err = parse (&p);
  ast_dump(p.root_node, stdout);
  if (err.code != SH_OK)
    {
      list_free (t.tokens, token_free);
      return err;
    }

  list_free (t.tokens, token_free);
  return err;
}
