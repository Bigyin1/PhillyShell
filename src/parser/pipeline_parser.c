#include "pipeline_parser.h"
#include "../errors/errors.h"
#include "cmd_parser.h"
#include "utils.h"

void *
pipeline_node_init (void *left, void *right)
{
  bin_op_node *bin_node = calloc (1, sizeof (bin_op_node));
  if (!bin_node)
    errors_fatal (MEM_ERROR);
  bin_node->type = NODE_PIPE;
  bin_node->token = SH_T_PIPE;
  bin_node->left = left;
  bin_node->right = right;
  return bin_node;
}

sh_ecode
parse_pipeline (sh_parser *p, void **res)
{
  void *node = NULL;
  sh_ecode err;

  err = parse_simple_cmd (p, (cmd_node **)&node);
  if (err != SH_OK)
    {
      parser_free (p);
      return err;
    }

  if (p->curr_token->type == SH_T_PIPE)
    {
      eat_token (p, SH_T_PIPE);

      void *right = NULL;
      if ((err = parse_pipeline (p, &right)) != SH_OK)
          return err;
      node = pipeline_node_init (node, right);
      eat_spaces (p);
    }
  *res = node;

  return SH_OK;
}
