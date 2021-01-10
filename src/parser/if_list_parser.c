//
// Created by sergey on 09.01.2021.
//

#include "if_list_parser.h"
#include "pipeline_parser.h"
#include "utils.h"

void *
if_node_init (sh_token *t)
{
  bin_op_node *bn = calloc (1, sizeof (bin_op_node));
  if (!bn)
    errors_fatal (MEM_ERROR);
  bn->type = NODE_IF;
  bn->token = t;
  return bn;
}

sh_ecode
parse_if_list (sh_parser *p, sh_ast_node *res)
{
  sh_ecode err;

  pipeline_node *pnl;
  err = parse_pipeline (p, &pnl);
  if (err != SH_OK)
    return err;
  *res = pnl;

  while (is_if_token (p->curr_token->type))
    {
      bin_op_node *bn = if_node_init (p->curr_token);
      eat_token (p, p->curr_token->type);

      pipeline_node *pnr;
      err = parse_pipeline (p, &pnr);
      if (err != SH_OK)
        return err;

      bn->left = *res;
      bn->right = pnr;
      *res = bn;
    }

  return SH_OK;
}
