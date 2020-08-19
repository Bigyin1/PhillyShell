#ifndef PHILLYSHELL_CMD_PARSER_H
#define PHILLYSHELL_CMD_PARSER_H

#include "cmd_tokenizer.h"
#include "structs/slice.h"
#include "structs/list.h"

typedef struct s_cmd_node {
  char *name;
  char **args;

} cmd_node;

typedef struct s_pipe_node
{
  List *cmd_nodes;
} pipe_node;

typedef struct s_list_cmd_node
{
  Slice *pipe_nodes;
} cmd_list_node;

#endif // PHILLYSHELL_CMD_PARSER_H
