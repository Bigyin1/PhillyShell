#include "parser.h"
#include "pipeline_parser.h"


sh_err parse (sh_parser *p) {

  sh_err err = {0};

  p->curr_token = get_next_token(p->tok);
  err = parse_pipeline(p, &p->root_node);

  return err;
}