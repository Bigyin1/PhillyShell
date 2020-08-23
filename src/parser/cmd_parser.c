#include "cmd_parser.h"
#include "../errors/errors.h"
#include "utils.h"

void
process_word (sh_parser *p, cmd_node *cn)
{
  char *word = p->curr_token->val;
  eat_token (p, SH_T_WORD);
  if (cn->name == NULL)
    {
      cn->name = word;
      return;
    }
  if (list_push_back (cn->args, word) != S_OK)
    errors_fatal (MEM_ERROR);
}

bool
is_fd (char *word)
{
  if (strlen (word) > 1)
    return false;
  if (word[0] > '9' || word[0] < '0')
    return false;
  return true;
}

sh_ecode
process_redir (sh_parser *p, cmd_node *cn)
{

  // 2>1
  int first_fd = -1;
  if (p->curr_token->type & SH_T_WORD)
    {
      if (!is_redir_token ((peek_next_token_type (p))))
        return SH_OK;
      if (!is_fd (p->curr_token->val))
        return SH_OK;
      first_fd = atoi (p->curr_token->val);
      eat_token (p, SH_T_WORD);
    }
  if (!is_redir_token ((p->curr_token->type)))
    return SH_OK;
  sh_token *redir_tok = p->curr_token;

  // proceeding to next token
  eat_token (p, redir_tok->type);
  eat_spaces (p);

  char *file = p->curr_token->val;
  if (eat_token (p, SH_T_WORD) != SH_OK)
    {
      printf ("fsh: parsing error near %s\n", redir_tok->val);
      return SH_ERR;
    }

  struct s_redir *redir = calloc (1, sizeof (struct s_redir));

  if (redir_tok->type & (SH_T_REDIR_S_R | SH_T_REDIR_D_R))
    {
      redir->from_fd = first_fd == -1 ? STDOUT_FILENO : first_fd;
      redir->to_file = file;
    }
  if (redir_tok->type & SH_T_REDIR_D_R)
    redir->append = true;
  if (redir_tok->type & SH_T_REDIR_S_L)
    {
      redir->to_fd = first_fd == -1 ? STDIN_FILENO : first_fd;
      redir->from_file = file;
    }
  if (list_push_back (cn->redirs, redir) != S_OK)
    errors_fatal (MEM_ERROR);

  return SH_OK;
}

// void
// process_var (sh_parser *p, cmd_node *cn)
//{
//  eat_token (p, SH_T_VAR_SIGN);
//  if (p->curr_token->type == SH_T_WORD)
//    {
//      char *val = hashtable_get (p->vars, p->curr_token->val);
//      eat_token (p, SH_T_WORD);
//      if (val != NULL)
//        process_word (cn, val);
//      return;
//    }
//  if (p->curr_token->type == SH_T_SPACE)
//    {
//      process_word (cn, "$");
//      return;
//    }
//}

cmd_node *
create_cmd_node (cmd_node **res)
{
  cmd_node *cn;

  *res = calloc (1, sizeof (cmd_node));
  if (!*res)
    errors_fatal (MEM_ERROR);
  cn = *res;
  cn->type = NODE_CMD;
  if (new_list (&cn->args) != S_OK)
    errors_fatal (MEM_ERROR);
  if (new_list (&cn->redirs) != S_OK)
    errors_fatal (MEM_ERROR);

  return cn;
}

sh_ecode
parse_simple_cmd (sh_parser *p, cmd_node **res)
{
  cmd_node *cn;
  int start = p->curr_token->pos;

  cn = create_cmd_node (res);
  eat_spaces (p);

  if (!(p->curr_token->type & SH_T_WORD))
    return SH_ERR;

  process_word (p, cn); // cmd name expected

  eat_spaces (p);
  for (; is_cmd_token (p->curr_token->type); eat_spaces (p))
    {
      if (process_redir (p, cn) != SH_OK)
        return SH_ERR;

      if (p->curr_token->type == SH_T_WORD)
        process_word (p, cn);
    }

  return SH_OK;
}