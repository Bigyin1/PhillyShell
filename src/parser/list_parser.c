//
// Created by sergey on 10.01.2021.
//

#include "list_parser.h"
#include "if_list_parser.h"
#include "utils.h"

list_node *
list_node_init (sh_token *t, sh_ast_node cont)
{
  list_node *bn = calloc (1, sizeof (list_node));
  if (!bn)
    errors_fatal (MEM_ERROR);
  bn->type = NODE_SEP;
  bn->token = t;
  bn->next = NULL;
  bn->cont = cont;
  return bn;
}

sh_ecode
parse_list (sh_parser *p, sh_ast_node *res)
{
  sh_ecode err;

  uint s = p->curr_token->pos;
  sh_ast_node cont;
  err = parse_if_list (p, &cont);
  if (err != SH_OK)
    return err;

  uint e = p->curr_token->pos;
  list_node *head = list_node_init (p->curr_token, cont);
  head->command = node_to_str(p->cmd, s, e);
  list_node *tail = head;

  while (is_sep_token (p->curr_token->type))
    {
      eat_token (p, p->curr_token->type);
      eat_spaces (p);
      s = p->curr_token->pos;

      if (p->curr_token->type == SH_T_EOF)
        break;
      err = parse_if_list (p, &cont);
      if (err != SH_OK)
        {
          ast_free (head);
          return err;
        }

      e = p->curr_token->pos;
      list_node *ln = list_node_init (p->curr_token, cont);
      ln->command = node_to_str(p->cmd, s, e);
      tail->next = ln;
      tail = ln;
    }
  *res = head;
  return SH_OK;
}