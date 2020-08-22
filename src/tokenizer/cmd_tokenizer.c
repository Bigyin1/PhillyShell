#include "cmd_tokenizer.h"
#include "../errors/errors.h"

char *reserved = "~$|<>&;\n\t ";
char *spaces = " \t\n";

sh_ecode
get_word (char *input, char **str, size_t *len, int pos)
{
  *len = strcspn (input, reserved);
  if (*len == 0)
    {
      printf (UNKNOWN_SYMBOL, pos);
      return SH_ERR;
    }
  *str = calloc (*len + 1, sizeof (char)); // TODO chek error
  strncpy (*str, input, *len);
  (*str)[*len] = '\0';
  return SH_OK;
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

void
token_free (void *token)
{
  sh_token *t = (sh_token *)token;

  free (t->val);
  free (t);
}

void
tokenizer_free (sh_tokenizer *t)
{
  list_free (t->tokens, token_free);
}

bool
check_token (sh_tokenizer *t, char **input, sh_token *tok,
             sh_token_type t_type, char *tok_val)
{
  size_t tok_len = strlen (tok_val);
  if (strncmp (*input, tok_val, tok_len) == 0)
    {
      tok->type = t_type;
      tok->val = strdup(tok_val);
      list_push_back (t->tokens, tok);
      (*input) += tok_len;
      return true;
    }
  return false;
}

sh_ecode
tokenize (sh_tokenizer *t, char *input)
{
  int pos = -1;
  sh_token *tok;
  size_t len;
  char *str;
  if (new_list (&t->tokens) != S_OK)
    errors_fatal (MEM_ERROR);
  while (*input)
    {
      ++pos;
      tok = calloc (1, sizeof (sh_token)); // TODO chek error
      tok->pos = pos;

      len = strspn (input, spaces);
      if (len != 0)
        {
          tok->type = SH_T_SPACE;
          tok->val = strdup(TOK_SPACE);
          list_push_back (t->tokens, tok);
          input += len;
          continue;
        }
      if (check_token (t, &input, tok, SH_T_PIPE, TOK_PIPE))
        continue;
      if (check_token (t, &input, tok, SH_T_SEMICOLON, TOK_SEMICOLON))
        continue;
      if (check_token (t, &input, tok, SH_T_TILDA, TOK_TILDA))
        continue;
      if (check_token (t, &input, tok, SH_T_VAR_SIGN, TOK_VAR_SIGN))
        continue;
      if (check_token (t, &input, tok, SH_T_REDIR_D_L, TOK_REDIR_D_L))
        continue;
      if (check_token (t, &input, tok, SH_T_REDIR_D_R, TOK_REDIR_D_R))
        continue;
      if (check_token (t, &input, tok, SH_T_REDIR_S_R, TOK_REDIR_S_R))
        continue;
      if (check_token (t, &input, tok, SH_T_REDIR_S_L, TOK_REDIR_S_L))
        continue;

      if (get_word (input, &str, &len, pos) != SH_OK)
        {
          free (tok);
          list_free (t->tokens, token_free);
          return SH_ERR;
        }
      tok->val = str;
      tok->type = SH_T_WORD;
      list_push_back (t->tokens, tok);
      input += len;
    }
  tok = calloc (1, sizeof (sh_token));
  tok->type = SH_T_EOF;
  list_push_back (t->tokens, tok);
  return SH_OK;
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
