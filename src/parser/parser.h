#pragma once

#include <stdbool.h>
#include <stdlib.h>
#include "../util/arraylist.h"

struct Parser;

#include "lexer.h"

DEFINE_ARRAYLIST(AST, struct Declaration*);

#define ERR_LOC_COLOR  COL_BLUE
#define ERR_ERR_COLOR  COL_RED
#define ERR_WARN_COLOR COL_MAGENTA

enum ParseErrorType {
  ERROR_UNREACHABLE,

  ERROR_UNIMPLEMENTED,

  ERROR_LEX_UNTERMINATED_STRING,
  ERROR_LEX_INVALID_CHAR_LITERAL,
  ERROR_LEX_INVALID_SYMBOL,

  ERROR_UNEXPECTED_EOF,

  ERROR_EXPECTED_DECLARATION,
  ERROR_EXPECTED_END_OF_DECLARATION,
  ERROR_EXPECTED_BLOCK,
  ERROR_EXPECTED_END_OF_BLOCK,
  ERROR_EXPECTED_EXPRESSION,
  ERROR_EXPECTED_TYPE,
  ERROR_EXPECTED_END_OF_STATEMENT,
  ERROR_EXPECTED_END_OF_VARIABLE,

  ERROR_EXPECTED_IDENTIFIER,
  ERROR_EXPECTED_LEFT_PAREN,
  ERROR_EXPECTED_RIGHT_PAREN,
  ERROR_EXPECTED_LEFT_CURLY,
  ERROR_EXPECTED_RIGHT_CURLY,
  ERROR_EXPECTED_LEFT_BRACKET,
  ERROR_EXPECTED_RIGHT_BRACKET,
  ERROR_EXPECTED_ASSIGN,
  ERROR_EXPECTED_STRING,

  ERROR_FINAL,
};

struct Parser {
  const char* filename;
  const char* program_index;
  size_t row, col;
  struct Token previous, current;
  bool isPanic, didPanic; // whether its currently panicking and if it ever did
};


void print_error(struct Parser*, enum ParseErrorType);
struct AST* parse_file(const char*);

#define RETURN_ERROR(parser, error) \
  ({ if(!(parser)->isPanic) print_error(parser, error); NULL; })

#define MATCH_TOKEN(parser, _type) \
  ((parser)->current.type == (TOKEN_##_type) ? \
   ({ (parser)->previous = (parser)->current; \
    (parser)->current = lex_token(parser); true; }) : false)

#define EXPECT_TOKEN(parser, _type, error) do { \
  if(MATCH_TOKEN(parser, ERROR)) \
    return RETURN_ERROR(parser, (parser)->current.as.integer); \
  if(!MATCH_TOKEN(parser, _type)) \
    return RETURN_ERROR(parser, ERROR_##error); \
  } while(0)

#define PRINT_INDENT(indent) \
  for(size_t jfkdla = 0; jfkdla < (indent); jfkdla++) printf("  ");
