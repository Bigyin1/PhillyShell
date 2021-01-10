#include "parser.h"

void ast_free (sh_ast_node node);

void
cmd_node_free (cmd_node *cn)
{
  if (!cn)
    return;
  list_free (cn->args, NULL);
  list_free (cn->redirs, free);
  free (cn);
}

void
bool_node_free (bin_op_node *bn)
{
  if (!bn)
    return;
  ast_free (bn->left);
  ast_free (bn->right);
  free (bn);
}

void
pipeline_node_free (pipeline_node *pn)
{
  if (!pn)
    return;
  list_free (pn->procs, (void (*) (void *))cmd_node_free);
  free (pn);
}

void
list_node_free (list_node *ln)
{
  if (!ln)
    return;

  ast_free (ln->cont);
  if (ln->next)
    list_node_free (ln->next);
  free (ln);
}

void
ast_free (sh_ast_node node)
{
  if (!node)
    return;
  node_type type = *(node_type *)node;

  if (type == NODE_SEP)
    list_node_free (node);

  if (type == NODE_CMD)
    cmd_node_free (node);

  if (type == NODE_IF)
    bool_node_free (node);

  if (type == NODE_PIPE)
    pipeline_node_free (node);
}

void
parser_free (sh_parser *p)
{
  ast_free (p->root_node);
  tokenizer_free (p->tok);
}
