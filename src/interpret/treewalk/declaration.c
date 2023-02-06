// declaration.c

#include "declaration.h"

static void walk_function(struct Function* ast) {
  return;
}

void walk_declaration(struct Declaration* ast) {
  switch(ast->type) {
    case DECL_VAR:
    case DECL_STRUCT:
    case DECL_UNION:
    case DECL_FUNC:   return walk_function(ast->as.function);
    case DECL_INC:
      break;
  }
}
