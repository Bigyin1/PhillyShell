#ifndef PHILLYSHELL_PARSER_H
#define PHILLYSHELL_PARSER_H

#include "../tokenizer/cmd_tokenizer.h"
#include "structs/hashtable.h"
#include <limits.h>

typedef void *sh_ast_node;

typedef enum e_node_type
{
  NODE_IF = 0,
  NODE_CMD = 1,
  NODE_PIPE = 2,
  NODE_SEP = 3,
} node_type;

struct s_redir
{
  int from_fd;
  int to_fd;
  char *to_file;
  char *from_file;
  bool append;
};

typedef struct s_cmd_node
{
  node_type type;
  char *name;
  List *args;
  List *redirs;

  char *command;
} cmd_node;

typedef struct s_pipeline_node
{
  node_type type;
  List *procs;
  char *command;
} pipeline_node;

typedef struct s_bin_op_node
{
  node_type type;
  sh_token *token;
  sh_ast_node left;
  sh_ast_node right;

} bin_op_node;

typedef struct s_list_node
{
  node_type type;
  sh_token *token;
  struct s_list_node *next;
  sh_ast_node *cont;
} list_node;

typedef struct s_parser
{
  char *cmd;
  // to be freed
  sh_tokenizer *tok;
  sh_ast_node root_node;

  sh_token *curr_token;
} sh_parser;

sh_ecode parse_tokens (sh_parser *p, sh_tokenizer *t);
void parser_free (sh_parser *p);
void ast_free (sh_ast_node node);
node_type get_node_type (sh_ast_node node);
void ast_dump (sh_ast_node node, FILE *f);

#endif // PHILLYSHELL_PARSER_H
