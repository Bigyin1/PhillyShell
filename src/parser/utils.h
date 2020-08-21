#ifndef PHILLYSHELL_UTILS_H
#define PHILLYSHELL_UTILS_H

#include "parser.h"

void eat_spaces (sh_parser *p);
sh_ecode eat_token (sh_parser *p, sh_token_type type);
sh_token_type peek_next_token_type (sh_parser *p);
void ast_free (void *node);
void ast_dump(void *node, FILE *f);

#endif // PHILLYSHELL_UTILS_H
