#include "pipeline_parser.h"
#include "cmd_parser.h"
#include "utils.h"

void *
pipeline_node_init (void *left, void *right)
{
  bin_op_node *bin_node = calloc (1, sizeof (bin_op_node));
  if (!bin_node)
    exit (EXIT_FAILURE);
  bin_node->type = NODE_BIN;
  bin_node->token = SH_T_PIPE;
  bin_node->left = left;
  bin_node->right = right;
  return bin_node;
}

sh_err
parse_pipeline (sh_parser *p, void **res)
{
  sh_err err;
  void *node = NULL;

  err = parse_simple_cmd (p, (cmd_node **)&node);
  if (err.code != SH_OK)
    return err;

  while (p->curr_token->type == SH_T_PIPE)
    {
      eat_token (p, SH_T_PIPE);

      void *right = NULL;
      err = parse_simple_cmd (p, (cmd_node **)&right);
      if (err.code != SH_OK)
        {
          ast_free (node);
          return err;
        }
      node = pipeline_node_init(node, right);
      eat_spaces(p);
    }
  *res = node;

  return err;
}
