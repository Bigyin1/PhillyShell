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
dump_cmd (cmd_node *cn, FILE *f)
{
  fprintf (f, "cmd: %s ", cn->name);
  Node *head = NULL;
  if (list_get_head (cn->args, &head) != S_OK)
    return;
  fprintf (f, "args: { ");
  while (head)
    {
      fprintf (f, "%s ", (char *)head->data);
      head = head->next;
    }
  fprintf (f, "} ");

  if (list_get_head (cn->redirs, &head) != S_OK)
    return;
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
}

void
dump_pipeline (pipeline_node *pn, FILE *f)
{
  Node *n;
  if (list_get_head (pn->procs, &n) == S_NOT_EXIST)
    return;
  for (; n; n = n->next)
    {
      dump_cmd (n->data, f);
      if (n->next)
        fprintf (f, " | ");
    }
}

void
dump_if_list (bin_op_node *bn, FILE *f)
{
  if (get_node_type (bn->left) == NODE_PIPE)
    dump_pipeline (bn->left, f);
  else
    dump_if_list (bn->left, f);

  fprintf (f, "if:%s ", bn->token->val);

  if (get_node_type (bn->right) == NODE_PIPE)
    dump_pipeline (bn->right, f);
  else
    dump_if_list (bn->right, f);
}

void
dump_list (list_node *ln, FILE *f)
{
  ast_dump (ln->cont, f);
  fprintf (f, "sep:%s ", ln->token->val);
  if (ln->next)
    dump_list (ln->next, f);
}

void
ast_dump (sh_ast_node node, FILE *f)
{
  if (!node)
    return;
  node_type type = get_node_type (node);
  if (type == NODE_SEP)
    dump_list (node, f);

  if (type == NODE_IF)
    dump_if_list (node, f);

  if (type == NODE_PIPE)
    dump_pipeline (node, f);
}
