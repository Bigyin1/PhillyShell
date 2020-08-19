#include "cmd_tokenizer.h"

extern char error_buf[512];
char *reserved = "~$|<>&;\n\t ";
char *spaces = " \t\n";

sh_err
get_string (char *input, char **str, size_t *len, int pos)
{
  sh_err err = { 0 };
  *len = strcspn (input, reserved);
  if (*len == 0)
    {
      err.code = SH_MODERATE;
      sprintf (error_buf, "fsh: unknown symbol at pos: %d\n", pos);
      return err;
    }
  *str = calloc (*len + 1, sizeof (char)); // TODO chek error
  strncpy (*str, input, *len);
  (*str)[*len] = '\0';
  return err;
}


void token_free(void *token) {
  sh_token *t = (sh_token*)token;

  free(t->val);
  free(t);
}

sh_err
tokenize (char *input, Slice *tokens)
{
  int pos = -1;
  sh_err err = { 0 };
  sh_token *tok;
  size_t len;
  char *str;
  while (*input)
    {
      ++pos;
      tok = calloc (1, sizeof (sh_token)); // TODO chek error
      tok->pos = pos;

      len = strspn (input, spaces);
      if (len != 0)
        {
          tok->type = SH_T_DELIM;
          slice_append (tokens, tok);
          input += len;
          continue;
        }
      if (strncmp (input, "|", 1) == 0)
        {
          tok->type = SH_T_PIPE;
          slice_append (tokens, tok);
          input++;
          continue;
        }
      if (strncmp (input, ";", 1) == 0)
        {
          tok->type = SH_T_SEMICOLON;
          slice_append (tokens, tok);
          input++;
          continue;
        }
      if (strncmp (input, "~", 1) == 0)
        {
          tok->type = SH_T_HOMEDIR;
          slice_append (tokens, tok);
          input++;
          continue;
        }
      if (strncmp (input, "$", 1) == 0)
        {
          input++;
          tok->type = SH_T_VAR;
          slice_append (tokens, tok);
          continue;
        }

      err = get_string (input, &str, &len, pos);
      if (err.code != SH_OK)
        {
          free (tok);
          return err;
        }
      tok->val = str;
      tok->type = SH_T_WORD;
      slice_append (tokens, tok); // TODO chek error
      input += len;
    }

  return err;
}

void tokenizer_dump(Slice *tokens, FILE *f) {
  for (int i = 0; i < slice_len(tokens); ++i)
    {
      sh_token *t;
      slice_get(tokens, i, (void **)&t);
      fprintf(f, "<%d>%s ", t->type, t->val);
    }
  fprintf(f, "\n");
}