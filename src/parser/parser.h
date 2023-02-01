#pragma once

#include <stdbool.h>
#include <stdlib.h>
#include "../util/arraylist.h"

struct Parser;

#include "lexer.h"

DEFINE_ARRAYLIST(AST, struct Expression*);

#define ERR_LOC_COLOR  COL_BLUE
#define ERR_ERR_COLOR  COL_RED
#define ERR_WARN_COLOR COL_MAGENTA

enum ParseErrorType {
  ERROR_UNDEFINED,
  ERROR_INVALID_DECLARATION,

  ERROR_EXPECTED_EXPRESSION,

  ERROR_EXPECTED_IDENTIFIER,
  ERROR_EXPECTED_LEFT_PAREN,
  ERROR_EXPECTED_RIGHT_PAREN,
  ERROR_EXPECTED_LEFT_CURLY,
  ERROR_EXPECTED_RIGHT_CURLY,
  ERROR_EXPECTED_ASSIGN,

  ERROR_FINAL,
};

struct Parser {
  const char* filename;
  size_t row, col; // TODO: make these update
  const char* program_index;
  struct Token previous;
  struct Token current;
  bool isPanic;
};


void print_error(struct Parser*, enum ParseErrorType);
struct AST* parse_file(const char*);

#define RETURN_ERROR(parser, error) \
  ({ print_error(parser, error); NULL; })

#define MATCH_TOKEN(parser, _type) \
  ((parser)->current.type == (TOKEN_##_type) ? \
   ({ (parser)->previous = (parser)->current; \
    (parser)->current = lex_token(parser); true; }) : false)

#define EXPECT_TOKEN(parser, _type, error) \
  if(!MATCH_TOKEN(parser, _type)) return RETURN_ERROR(parser, error)

#define PRINT_INDENT(indent) \
  for(size_t jfkdla = 0; jfkdla < (indent); jfkdla++) printf(" ");
