// interpreter.c

#include "ctx.h"
#include "interpreter.h"
#include "declaration.h"

void walk_tree(struct AST* ast) {
  struct Interpreter interpreter;
  NEW_ARRAYLIST(&interpreter.frames);

  for(size_t i = 0; i < ast->size; i++) {
    walk_declaration(ast->members[i], &interpreter);
  }
}
