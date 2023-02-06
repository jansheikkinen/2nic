// expression.c

#include "expression.h"
#include <stdio.h>
#include <string.h>


static struct Value* walk_literal(struct Expression* ast,
    struct Interpreter* ctx) {
  struct Value* value = malloc(sizeof(*value));

  value->type = ast->as.literal.type;
  value->as = ast->as.literal.as;

  return value;
}


static struct Value* walk_binary(struct Binary* ast,
    struct Interpreter* ctx) {
  struct Value* left = walk_expression(ast->left, ctx);
  struct Value* right = walk_expression(ast->right, ctx);

  switch(ast->op) {
    case TOKEN_LOGIC_AND:
    case TOKEN_LOGIC_OR:

    case TOKEN_CATCH:
    case TOKEN_ORELSE:
    case TOKEN_AS:

    case TOKEN_NOT_EQ:
    case TOKEN_EQ:

    case TOKEN_LT:
    case TOKEN_LT_EQ:
    case TOKEN_GT:
    case TOKEN_GT_EQ:

    case TOKEN_BIT_SHL:
    case TOKEN_BIT_SHR:
    case TOKEN_BIT_AND:
    case TOKEN_BIT_OR:
    case TOKEN_BIT_XOR:

    case TOKEN_ADD:
      if(MATCH_VAL(left, INT) && MATCH_VAL(right, INT)) {
        size_t x = FROM_INT(left) + FROM_INT(right);
        free(left); free(right);

        return alloc_int_val(x);
      }
      break;
    case TOKEN_ADD_WRAP:
    case TOKEN_SUB:
    case TOKEN_SUB_WRAP:

    case TOKEN_MUL:
    case TOKEN_MUL_WRAP:
    case TOKEN_DIV:
    case TOKEN_MOD:

    // should probably delete this
    case TOKEN_LOGIC_XOR:
    default: break;
  }

  return NULL;
}


static struct Value* walk_print(struct Value* args,
    struct Interpreter* ctx) {
  switch(args->type) {

  case VAL_UNDEFINED:
  case VAL_BOOL:
  case VAL_INT:
    printf("%zu\n", args->as.integer); break;
  case VAL_FLOAT:
  case VAL_CHAR:
  case VAL_STRING:
  case VAL_IDENTIFIER:
  case VAL_PTR:
      break;
  }
}

static struct Value* walk_call(struct Call* ast, struct Interpreter* ctx) {
  struct Value* callee = walk_expression(ast->callee, ctx);
  struct Value* arguments = walk_expression(ast->arguments, ctx);

  if(MATCH_VAL(callee, IDENTIFIER) && !strcmp(callee->as.string, "print")) {
    free(callee);
    walk_print(arguments, ctx);
  }

  return NULL;
}


struct Value* walk_expression(struct Expression* ast, struct Interpreter* ctx) {
  switch(ast->type) {
  case EXPR_LITERAL:     return walk_literal(ast, ctx);
  case EXPR_UNARY:
  case EXPR_BINARY:      return walk_binary(&ast->as.binary, ctx);
  case EXPR_GROUP:
  case EXPR_CALL:        return walk_call(&ast->as.call, ctx);
  case EXPR_FIELD:
  case EXPR_ARRAY_INDEX:
  case EXPR_ARRAY_INIT:
  case EXPR_CAST:
  case EXPR_ASSIGN:
  case EXPR_LIST:
  case EXPR_BLOCK:       return walk_block(&ast->as.block, ctx);
  case EXPR_IF:
  case EXPR_WHILE:
    break;
  }
}


static void walk_statement(struct Statement* ast, struct Interpreter* ctx) {
  switch(ast->type) {
    case STMT_EXPR:  walk_expression(ast->as.expr, ctx); return;
    case STMT_BLOCK:
    case STMT_VAR:
      break;
  }
}


static void walk_statements(struct StatementList ast, struct Interpreter* ctx) {
  for(size_t i = 0; i < ast.size; i++) walk_statement(&ast.members[i], ctx);
}

struct Value* walk_block(struct Block* ast, struct Interpreter* ctx) {
  walk_statements(ast->stmts, ctx);

  if(ast->expr) return walk_expression(ast->expr, ctx);
  return NULL;
}
