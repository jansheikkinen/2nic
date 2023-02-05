#pragma once

#include "parser.h"

enum TypeType {
  TYPE_PRIMITIVE,
  TYPE_WRAPPER,
  TYPE_ARRAY,
  TYPE_COMPOUND,
};

struct Wrapper {
  enum TokenType op;
  struct Type* type;
};

struct Array {
  struct Expression* size;
  struct Type* type;
};

struct Compound {
  enum { COMP_STRUCT, COMP_UNION, COMP_FUNC } type;
  union {
    struct Struct* _struct;
    struct Union* _union;
    struct FuncSig* sig;
  } as;
};

struct Type {
  enum TypeType type; // lol
  bool is_mutable;
  union {
    enum TokenType primitive;
    struct Wrapper wrapper;
    struct Array array;
    struct Compound compound;
  } as;
};

struct Type* parse_type(struct Parser*);
void print_type(const struct Type*);
