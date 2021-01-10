#include "parser.h"
#include "list_parser.h"

sh_ecode
parse_tokens (sh_parser *p, sh_tokenizer *t)
{
  p->tok = t;
  p->curr_token = get_next_token (p->tok);
  sh_ecode e = parse_list (p, &p->root_node);
  return e;
}