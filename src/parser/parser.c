#include "parser.h"
#include "pipeline_parser.h"

sh_ecode
parse_tokens (sh_parser *p)
{
  p->curr_token = get_next_token (p->tok);
  return parse_pipeline (p, &p->root_node);
}