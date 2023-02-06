// interpreter.c

#include "interpreter.h"
#include "declaration.h"

void walk_tree(struct AST* ast) {
  for(size_t i = 0; i < ast->size; i++) walk_declaration(ast->members[i]);
  return;
}
