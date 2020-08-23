#ifndef PHILLYSHELL_CMD_TOKENIZER_H
#define PHILLYSHELL_CMD_TOKENIZER_H
#include "../shell.h"
#include "structs/list.h"
#include <stdbool.h>

typedef enum e_sh_token_type
{
  // simple cmd tokens set
  SH_T_WORD = 1 << 0,
  SH_T_REDIR_S_R = 1 << 1,
  SH_T_REDIR_D_R = 1 << 2,
  SH_T_REDIR_S_L = 1 << 3,
  SH_T_REDIR_D_L = 1 << 4,
  SH_T_TILDA = 1 << 5,
  SH_T_VAR_SIGN = 1 << 6,

  SH_T_PIPE = 1 << 7,

  SH_T_SEMICOLON = 1 << 8,

  SH_T_SPACE = 1 << 9,
  SH_T_EOF = 1 << 10,
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

#define cmd_tokens_mask                                                       \
  (SH_T_WORD | SH_T_VAR_SIGN | SH_T_TILDA | SH_T_REDIR_D_L | SH_T_REDIR_D_R   \
   | SH_T_REDIR_S_L | SH_T_REDIR_S_R)

#define is_cmd_token(t_type) t_type &cmd_tokens_mask

#define redir_tokens                                                          \
  (SH_T_REDIR_D_L | SH_T_REDIR_D_R | SH_T_REDIR_S_L | SH_T_REDIR_S_R)

#define is_redir_token(t_type) t_type &redir_tokens

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
