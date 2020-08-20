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

sh_token *
get_next_token (sh_tokenizer *t)
{
  if (t->_curr == NULL)
    {
      if (list_get_head (t->tokens, &t->_curr) != S_OK)
        return NULL;
    }
  sh_token *tok = t->_curr->data;
  t->_curr = t->_curr->next;
  return tok;
}

sh_err
tokenize (sh_tokenizer *t, char *input)
{
  int pos = -1;
  sh_err err = { 0 };
  sh_token *tok;
  size_t len;
  char *str;
  if (new_list (&t->tokens) != S_OK)
    {
      err.code = SH_FATAL;
      return err;
    }
  while (*input)
    {
      ++pos;
      tok = calloc (1, sizeof (sh_token)); // TODO chek error
      tok->pos = pos;

      len = strspn (input, spaces);
      if (len != 0)
        {
          tok->type = SH_T_SPACE;
          list_push_back (t->tokens, tok);
          input += len;
          continue;
        }
      if (strncmp (input, "|", 1) == 0)
        {
          tok->type = SH_T_PIPE;
          list_push_back (t->tokens, tok);
          input++;
          continue;
        }
      if (strncmp (input, ";", 1) == 0)
        {
          tok->type = SH_T_SEMICOLON;
          list_push_back (t->tokens, tok);
          input++;
          continue;
        }
      if (strncmp (input, "~", 1) == 0)
        {
          tok->type = SH_T_HOMEDIR;
          list_push_back (t->tokens, tok);
          input++;
          continue;
        }
      if (strncmp (input, "$", 1) == 0)
        {
          input++;
          tok->type = SH_T_VAR;
          list_push_back (t->tokens, tok);
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
      list_push_back (t->tokens, tok);
      input += len;
    }
  tok = calloc (1, sizeof (sh_token));
  tok->type = SH_T_EOF;
  list_push_back (t->tokens, tok);
  return err;
}

void
tokenizer_dump (sh_tokenizer *t, FILE *f)
{
  Node *n = NULL;
  list_get_head (t->tokens, &n);
  while (n)
    {
      sh_token *tok = n->data;
      fprintf (f, "<%d>%s ", tok->type, tok->val);
      n = n->next;
    }
  fprintf (f, "\n");
}

void
token_free (void *token)
{
  sh_token *t = (sh_token *)token;

  free (t->val);
  free (t);
}