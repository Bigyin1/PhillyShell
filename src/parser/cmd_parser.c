#include "cmd_parser.h"
#include "../errors/errors.h"
#include "utils.h"

extern char error_buf[512];

void
process_word (cmd_node *cn, char *word)
{
  if (cn->name == NULL)
    {
      cn->name = word;
      return;
    }
  if (list_push_back (cn->args, word) != S_OK)
    exit (EXIT_FAILURE);
}

void
process_var (sh_parser *p, cmd_node *cn)
{
  eat_token (p, SH_T_VAR);
  if (p->curr_token->type == SH_T_WORD)
    {
      char *val = hashtable_get (p->vars, p->curr_token->val);
      eat_token (p, SH_T_WORD);
      if (val != NULL)
        process_word (cn, val);
      return;
    }
  if (p->curr_token->type == SH_T_SPACE)
    {
      process_word (cn, "$");
      return;
    }
}

bool
is_cmd_token (sh_parser *p)
{
  return p->curr_token->type == SH_T_WORD || p->curr_token->type == SH_T_VAR
         || p->curr_token->type == SH_T_HOMEDIR;
}

sh_ecode
parse_simple_cmd (sh_parser *p, cmd_node **res)
{
  cmd_node *cn;
  int start = p->curr_token->pos;

  *res = calloc (1, sizeof (cmd_node));
  if (!*res)
    errors_fatal (MEM_ERROR);
  cn = *res;
  cn->type = NODE_CMD;
  if (new_list (&cn->args) != S_OK)
    errors_fatal (MEM_ERROR);

  eat_spaces (p);
  while (is_cmd_token (p))
    {
      if (p->curr_token->type == SH_T_WORD)
        {
          process_word (cn, p->curr_token->val);
          eat_token (p, SH_T_WORD);
          eat_spaces (p);
        }
      if (p->curr_token->type == SH_T_VAR)
        process_var (p, cn);
    }
  if (cn->name == NULL)
    {
      printf (EMPTY_CMD, start);
      return SH_ERR;
    }

  return SH_OK;
}