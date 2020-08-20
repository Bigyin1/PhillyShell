#include "cmd_tokenizer.h"

bool
is_pipeline_delim (sh_token_type type)
{
  if (type == SH_T_SEMICOLON)
    return true;
  return false;
}