#include "utils.h"
#include "parser.h"

extern char error_buf[512];

void
cmd_node_free (cmd_node *cn)
{
  if (!cn)
    return;
  list_free (cn->args, NULL);
  free (cn);
}

void
pipe_node_free (bin_op_node *pn)
{
  if (!pn)
    return;
  ast_free (pn->left);
  ast_free (pn->right);
  free (pn);
}

void
ast_free (void *node)
{
  if (!node)
    return;
  node_type type = *(node_type *)node;
  if (type == NODE_CMD)
    cmd_node_free (node);

  if (type == NODE_BIN)
    pipe_node_free (node);
}



void ast_dump(void *node, FILE *f) {
  if (!node)
    return;
  node_type type = *(node_type *)node;
  if (type == NODE_CMD) {
    cmd_node *cn = node;
    fprintf(f, "cmd: %s ", cn->name);
    Node *head = NULL;
    if (list_get_head(cn->args, &head) != S_OK) return;
    fprintf(f, "args: { ");
    while (head) {
      fprintf(f, "%s ", head->data);
      head = head->next;
    }
    fprintf(f, "}");
    return;
  }
  if (type == NODE_BIN) {
    bin_op_node *cn = node;
    ast_dump(cn->left, f);
    if (cn->token == SH_T_PIPE) {
      fprintf(f, " | ");
    }
    ast_dump(cn->right, f);
  }
}

sh_err
eat_token (sh_parser *p, sh_token_type type)
{
  sh_err err = { 0 };

  if (p->curr_token->type != type)
    {
      err.code = SH_MODERATE;
      sprintf (error_buf, "fsh: parse error at pos: %d\n", p->curr_token->pos);
      return err;
    }
  p->curr_token = get_next_token (p->tok);
  return err;
}

sh_token_type
peek_next_token_type (sh_parser *p)
{
  sh_token *t = (sh_token *)p->tok->_curr;
  return t->type;
}

void
eat_spaces (sh_parser *p)
{
  while (p->curr_token->type == SH_T_SPACE)
    {
      eat_token (p, SH_T_SPACE);
    }
}
