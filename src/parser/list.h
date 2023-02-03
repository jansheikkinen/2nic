#pragma once

#include "parser.h"

struct LValue {
  const char* name;
  struct Type* type;
};

struct VarDecl {
  struct LValue* lvalue;
  struct Expression* expr;
};


#define DEF_LINKED_LIST(name, _type) \
  struct name { _type current; struct name* next; }

DEF_LINKED_LIST(ExprList, struct Expression*);
DEF_LINKED_LIST(VarDeclList, struct VarDecl*);
DEF_LINKED_LIST(TypeList, struct Type*);

struct ExprList* parse_exprs(struct Parser*);
struct VarDeclList* parse_vardecls(struct Parser*);
struct TypeList* parse_types(struct Parser*);

void print_exprs(const struct ExprList*);
void print_vardelcs(const struct VarDeclList*);
void print_types(const struct TypeList*);
