#pragma once

#include "parser.h"
#include "list.h"

struct Variable {
  struct VarDeclList* vars;
};

struct Variable* parse_variable(struct Parser*);
void print_variable(const struct Variable*);
