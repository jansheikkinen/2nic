#pragma once

#include "parser.h"
#include "list.h"

struct Variable {
  struct VarDeclList* vars;
};

struct Struct {
  const char* name;
  struct VarDeclList* fields;
};

struct Union {
  const char* name;
  struct TypeList* fields;
};

struct Enum {
  const char* name;
  struct AssignList* fields;
};

struct FuncSig {
  const char* name;
  struct VarDeclList* args;
  struct TypeList* returns;
};

struct Function {
  struct FuncSig* sig;
  struct Block* body;
};

struct Declaration {
  enum { DECL_VAR, DECL_STRUCT, DECL_UNION, DECL_ENUM, DECL_FUNC } type;
  union {
    struct Variable* var;
    struct Struct* _struct;
    struct Union* _union;
    struct Enum* _enum;
    struct Function* function;
  } as;
};

struct Variable* parse_variable(struct Parser*);
struct Declaration* parse_declaration(struct Parser*);

void print_variable(const struct Variable*);
void print_declaration(const struct Declaration*);