#include "exec_cmd.h"
#include "cmd_tokenizer.h"

extern char error_buf[512];

sh_err
exec_cmd (Shell *sh)
{
  sh_err err = { 0 };
  Slice *tokens;
  if (slice_new (&tokens, 0, 32) != S_OK)
    {
      err.code = SH_FATAL;
      return err;
    }
  tokenize(sh->cmd_buf, tokens);

  slice_free(tokens, token_free);
  return err;
}
