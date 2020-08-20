#include "cmd_parser.h"
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
      if (val != NULL)
        {
          process_word (cn, val);
          return;
        }
    }
  if (p->curr_token->type == SH_T_SPACE)
    {
      process_word (cn, "$");
      return;
    }
}

sh_err
parse_simple_cmd (sh_parser *p, cmd_node **res)
{
  sh_err err = { 0 };
  cmd_node *cn;
  int start = p->curr_token->pos;

  *res = calloc (1, sizeof (cmd_node));
  if (!*res)
    exit (EXIT_FAILURE);
  cn = *res;
  cn->type = NODE_CMD;
  if (new_list (&cn->args) != S_OK)
    exit (EXIT_FAILURE);

  eat_spaces (p);
  while (p->curr_token->type == SH_T_WORD || p->curr_token->type == SH_T_VAR
         || p->curr_token->type == SH_T_HOMEDIR)
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
      sprintf (error_buf, "command at pos %d is empty", start);
      err.err = error_buf;
      err.code = SH_MODERATE;
      return err;
    }

  return err;
}