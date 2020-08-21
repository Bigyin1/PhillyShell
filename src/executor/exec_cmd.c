#include "exec_cmd.h"
#include "../parser/parser.h"
#include "../parser/utils.h"
#include "../tokenizer/cmd_tokenizer.h"

sh_ecode
exec_cmd (Shell *sh)
{
  sh_tokenizer t = { 0 };
  sh_parser p = { 0 };

  if (tokenize (&t, sh->cmd_buf) != SH_OK)
    {
      list_free (t.tokens, token_free);
      return SH_ERR;
    }

  p.tok = &t;
  p.vars = sh->env;
  if (parse (&p) != SH_OK)
    {
      list_free (t.tokens, token_free);
      return SH_ERR;
    }
  ast_dump (p.root_node, stdout);
  list_free (t.tokens, token_free);
  return SH_OK;
}
