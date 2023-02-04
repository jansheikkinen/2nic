#pragma once

#include "parser.h"

enum TypeType {
  TYPE_PRIMITIVE,
  TYPE_RESULT,
  TYPE_OPTIONAL,
  TYPE_REFERENCE,
  TYPE_COMPOUND,
};

struct Type {
  enum TypeType type; // lol
  union {
    enum TokenType primitive;
  } as;
};

struct Type* parse_type(struct Parser*);
void print_type(const struct Type*);
