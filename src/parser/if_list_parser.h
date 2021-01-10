//
// Created by sergey on 09.01.2021.
//

#ifndef PHILLYSHELL_IF_LIST_PARSER_H
#define PHILLYSHELL_IF_LIST_PARSER_H

#include "../tokenizer/cmd_tokenizer.h"
#include "parser.h"

sh_ecode
parse_if_list (sh_parser *p, sh_ast_node *res);

#endif // PHILLYSHELL_IF_LIST_PARSER_H
