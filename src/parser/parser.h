#pragma once

#include <stdbool.h>
#include "../common.h"
#include "../util/arraylist.h"

struct Parser;

#include "lexer.h"

DEFINE_ARRAYLIST(AST, struct Declaration*);

enum ParseErrorType {
  ERROR_UNDEFINED,
  ERROR_INVALID_DECLARATION,

  ERROR_EXPECTED_IDENTIFIER,
  ERROR_EXPECTED_LEFT_PAREN,
  ERROR_EXPECTED_RIGHT_PAREN,
  ERROR_EXPECTED_LEFT_CURLY,
  ERROR_EXPECTED_RIGHT_CURLY,
  ERROR_EXPECTED_ASSIGN,

};

struct ParseError {
  const char* error_file;
  size_t row, col;
  enum ParseErrorType type;
};

DEFINE_ARRAYLIST(ParseErrorList, struct ParseError);

struct Parser {
  const char* filename;
  size_t row, col; // TODO: make these update
  const char* program_index;
  struct ParseErrorList errors;
  struct Token token;
  bool isPanic;
};


void append_error(struct Parser*, enum ParseErrorType);
struct AST* parse_file(const char*);

#define RETURN_ERROR(parser, error) \
  ({ append_error(parser, error); NULL; })

#define MATCH_TOKEN(parser, token, _type) \
  (((token) = lex_token(parser)).type == (_type))

#define EXPECT_TOKEN(parser, token, _type, error) \
  if(!MATCH_TOKEN(parser, token, _type)) return RETURN_ERROR(parser, error)
