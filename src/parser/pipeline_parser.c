#include "pipeline_parser.h"
#include "cmd_parser.h"
#include "utils.h"

void *
pipeline_node_init ()
{
  pipeline_node *pn = calloc (1, sizeof (pipeline_node));
  if (!pn)
    errors_fatal (MEM_ERROR);
  pn->type = NODE_PIPE;
  if (new_list (&pn->procs) != S_OK)
    errors_fatal (MEM_ERROR);
  return pn;
}

sh_ecode
parse_pipeline (sh_parser *p, pipeline_node **res)
{
  pipeline_node *pn = pipeline_node_init ();
  sh_ecode err;

  cmd_node *cn;
  err = parse_simple_cmd (p, &cn);
  if (err != SH_OK)
    {
      ast_free(pn);
      return err;
    }
  list_push_back (pn->procs, cn);

  if (p->curr_token->type == SH_T_PIPE)
    {
      eat_token (p, SH_T_PIPE);

      err = parse_simple_cmd (p, &cn);
      if (err != SH_OK)
        {
          ast_free(pn);
          return err;
        }
      list_push_back (pn->procs, cn);
      eat_spaces (p);
    }

  *res = pn;
  return SH_OK;
}
