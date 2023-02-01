#pragma once

#include "parser.h"

enum LiteralType {
  LIT_BOOL, LIT_INT, LIT_FLOAT, LIT_CHAR, LIT_STRING, LIT_IDENTIFIER
};

struct Literal {
  enum LiteralType type;
  union {
    const char* string;
    size_t integer;
    double floating;
    char character;
    bool boolean;
  } as;
};

struct Unary {
  struct Expression* operand;
  enum TokenType op;
};

struct Binary {
  struct Expression* left;
  struct Expression* right;
  enum TokenType op;
};

struct Expression {
  enum { EXPR_LITERAL, EXPR_UNARY, EXPR_BINARY } type;
  union {
    struct Binary binary;
    struct Unary unary;
    struct Literal literal;
  } as;
};

struct Expression* parse_expression(struct Parser*);
void print_expression(const struct Expression*, size_t);
