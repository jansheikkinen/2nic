// declaration.c

#include "expression.h"
#include "declaration.h"

static void walk_function(struct Function* ast, struct Interpreter* ctx) {
  push_frame(ctx, ast->sig->name ? : "<anonymous function>");

  // set the arguments to values in the hashmap
  // walk_vardecls(ctx, ast->sig->args);

  struct Value* returned = walk_block(ast->body, ctx);
  // TODO: typecheck returned value and return it if its poggers

  pop_frame(ctx);
}

void walk_declaration(struct Declaration* ast, struct Interpreter* ctx) {
  switch(ast->type) {
    case DECL_VAR:
    case DECL_STRUCT:
    case DECL_UNION:
    case DECL_FUNC:   return walk_function(ast->as.function, ctx);
    case DECL_INC:
      break;
  }
}
