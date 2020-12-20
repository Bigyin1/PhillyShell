#include "utils.h"
#include "../errors/errors.h"
#include "parser.h"

sh_ecode
eat_token (sh_parser *p, sh_token_type type)
{
  if (p->curr_token->type != type)
    {
      printf (PARSE_ERROR, p->curr_token->pos);
      return SH_ERR;
    }
  p->curr_token = get_next_token (p->tok);
  return SH_OK;
}

void
eat_spaces (sh_parser *p)
{
  while (p->curr_token->type == SH_T_SPACE)
    {
      eat_token (p, SH_T_SPACE);
    }
}

sh_token_type
peek_next_token_type (sh_parser *p)
{
  sh_token *t = (sh_token *)p->tok->_curr;
  return t->type;
}

node_type
get_node_type (sh_ast_node node)
{
  return *(node_type *)node;
}

void
ast_dump (sh_ast_node node, FILE *f)
{
  if (!node)
    return;
  node_type type = get_node_type (node);
  if (type == NODE_CMD)
    {
      cmd_node *cn = node;
      fprintf (f, "cmd: %s ", cn->name);
      Node *head = NULL;
      if (list_get_head (cn->args, &head) != S_OK)
        {
          //fprintf(f, "\n");
          return;
        }
      fprintf (f, "args: { ");
      while (head)
        {
          fprintf (f, "%s ", (char *)head->data);
          head = head->next;
        }
      fprintf (f, "}");

      if (list_get_head (cn->redirs, &head) != S_OK)
        {
          //fprintf(f, "\n");
          return;
        }
      fprintf (f, " redirs: [ ");
      while (head)
        {
          struct s_redir *r = head->data;
          if (r->from_file)
            fprintf (f, " %d<%s ", r->to_fd, r->from_file);
          if (r->to_file)
            r->append ? fprintf (f, " %d>>%s ", r->from_fd, r->to_file)
                      : fprintf (f, " %d>%s ", r->from_fd, r->to_file);
          head = head->next;
        }
      fprintf (f, "]");
      //fprintf(f, "\n");
      return;
    }
  if (type == NODE_PIPE)
    {
      bin_op_node *cn = node;
      ast_dump (cn->left, f);
      fprintf (f, " | ");
      ast_dump (cn->right, f);
    }
}
