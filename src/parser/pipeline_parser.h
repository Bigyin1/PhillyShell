#ifndef PHILLYSHELL_PIPELINE_PARSER_H
#define PHILLYSHELL_PIPELINE_PARSER_H

#include "../tokenizer/cmd_tokenizer.h"
#include "parser.h"

sh_ecode parse_pipeline (sh_parser *p, pipeline_node **res);

#endif // PHILLYSHELL_PIPELINE_PARSER_H
