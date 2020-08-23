#include "parser.h"

void
ast_free (sh_ast_node node);

void
cmd_node_free (cmd_node *cn)
{
  if (!cn)
    return;
  list_free (cn->args, NULL);
  list_free(cn->redirs, free);
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
ast_free (sh_ast_node node)
{
  if (!node)
    return;
  node_type type = *(node_type *)node;
  if (type == NODE_CMD)
    cmd_node_free (node);

  if (type == NODE_BOOL || type == NODE_PIPE)
    pipe_node_free (node);
}

void parser_free(sh_parser *p) {
    ast_free(p->root_node);
    tokenizer_free(p->tok);
}
