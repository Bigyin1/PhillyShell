//
// Created by sergey on 10.01.2021.
//

#ifndef PHILLYSHELL_LIST_PARSER_H
#define PHILLYSHELL_LIST_PARSER_H

#include "../tokenizer/cmd_tokenizer.h"
#include "parser.h"

sh_ecode
parse_list (sh_parser *p, sh_ast_node *res);

#endif // PHILLYSHELL_LIST_PARSER_H
