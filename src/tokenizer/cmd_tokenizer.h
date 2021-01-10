#ifndef PHILLYSHELL_CMD_TOKENIZER_H
#define PHILLYSHELL_CMD_TOKENIZER_H
#include "../errors/errors.h"
#include "structs/list.h"
#include <stdbool.h>
#include <stdio.h>

typedef enum e_sh_token_type
{
  // simple cmd tokens set
  SH_T_WORD = 1U << 0,
  SH_T_REDIR_S_R = 1U << 1,
  SH_T_REDIR_D_R = 1U << 2,
  SH_T_REDIR_S_L = 1U << 3,
  SH_T_REDIR_D_L = 1U << 4,
  SH_T_TILDA = 1U << 5,
  SH_T_VAR_SIGN = 1U << 6,

  SH_T_PIPE = 1U << 7,

  SH_T_SEMICOLON = 1U << 8,

  SH_T_SPACE = 1U << 9,
  SH_T_EOF = 1U << 10,
  SH_T_AMP = 1U << 11,
  SH_T_AND_IF = 1U << 12,
  SH_T_OR_IF = 1U << 13,

} sh_token_type;

#define TOK_PIPE "|"
#define TOK_SEMICOLON ";"
#define TOK_REDIR_S_R ">"
#define TOK_REDIR_D_R ">>"
#define TOK_REDIR_S_L "<"
#define TOK_REDIR_D_L "<<"
#define TOK_TILDA "~"
#define TOK_VAR_SIGN "$"
#define TOK_SPACE " "
#define TOK_AMP "&"
#define TOK_AND_IF "&&"
#define TOK_OR_IF "||"

#define cmd_tokens_mask                                                       \
  (SH_T_WORD | SH_T_VAR_SIGN | SH_T_TILDA | SH_T_REDIR_D_L | SH_T_REDIR_D_R   \
   | SH_T_REDIR_S_L | SH_T_REDIR_S_R)
#define is_cmd_token(t_type) ((t_type)&cmd_tokens_mask)

#define redir_tokens                                                          \
  (SH_T_REDIR_D_L | SH_T_REDIR_D_R | SH_T_REDIR_S_L | SH_T_REDIR_S_R)
#define is_redir_token(t_type) ((t_type)&redir_tokens)

#define if_tokens (SH_T_AND_IF | SH_T_OR_IF)
#define is_if_token(t_type) ((t_type)&if_tokens)

#define sep_tokens (SH_T_SEMICOLON | SH_T_AMP)
#define is_sep_token(t_type) ((t_type)&sep_tokens)

typedef struct s_sh_token
{
  char *val;
  sh_token_type type;
  int pos;
} sh_token;

typedef struct s_tokenizer
{
  List *tokens;
  Node *_curr;
} sh_tokenizer;

sh_ecode tokenize (sh_tokenizer *t, char *input);
sh_token *get_next_token (sh_tokenizer *t);
void tokenizer_free (sh_tokenizer *t);
void tokenizer_dump (sh_tokenizer *t, FILE *f);

#endif // PHILLYSHELL_CMD_TOKENIZER_H
