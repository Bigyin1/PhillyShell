#ifndef PHILLYSHELL_PARSER_H
#define PHILLYSHELL_PARSER_H

#include "../tokenizer/cmd_tokenizer.h"

typedef void *sh_ast_node;

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
  sh_ast_node left;
  sh_ast_node right;

} bin_op_node;

typedef struct s_parser
{
  // to be freed
  sh_tokenizer *tok;
  sh_ast_node root_node;


  sh_token *curr_token;
  HTable *vars;
} sh_parser;

sh_ecode parse_tokens (sh_parser *p);
void parser_free(sh_parser *p);
node_type get_node_type (sh_ast_node node);

#endif // PHILLYSHELL_PARSER_H
