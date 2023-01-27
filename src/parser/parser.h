#ifndef PARSER_PARSER_H
#define PARSER_PARSER_H

#include "lexer.h"
#include "../common.h"

struct Parser {
  const char* filename;
  const char* program_index;
  struct AST* ast;
};

struct AST* parse_file(const char*);

#endif
