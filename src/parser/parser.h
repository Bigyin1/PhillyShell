#ifndef PHILLYSHELL_PARSER_H
#define PHILLYSHELL_PARSER_H

#include "../tokenizer/cmd_tokenizer.h"

typedef enum e_node_type
{
  NODE_BIN = 0,
  NODE_CMD = 1,
} node_type;

typedef struct s_cmd_node
{
  node_type type;
  char *name;
  List *args;

} cmd_node;

typedef struct s_bin_op_node
{
  node_type type;
  sh_token_type token;
  void *left;
  void *right;

} bin_op_node;

typedef struct s_parser
{
  sh_tokenizer *tok;
  sh_token *curr_token;
  HTable *vars;
  void *root_node;
} sh_parser;

sh_ecode parse (sh_parser *p);

#endif // PHILLYSHELL_PARSER_H
