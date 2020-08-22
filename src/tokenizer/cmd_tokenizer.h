#ifndef PHILLYSHELL_CMD_TOKENIZER_H
#define PHILLYSHELL_CMD_TOKENIZER_H
#include "../shell.h"
#include "structs/list.h"
#include <stdbool.h>

typedef enum e_sh_token_type
{
  SH_T_WORD = 0,
  SH_T_PIPE = 1,
  SH_T_SEMICOLON = 2,
  SH_T_REDIR_S_R = 3,
  SH_T_REDIR_D_R = 4,
  SH_T_REDIR_S_L = 5,
  SH_T_REDIR_D_L = 6,
  SH_T_TILDA = 7,
  SH_T_VAR_SIGN = 8,
  SH_T_SPACE = 9,
  SH_T_EOF = 10,
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
