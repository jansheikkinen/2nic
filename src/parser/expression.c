// expression.c

#include "expression.h"
#include <stdio.h>


// ### ALLOCATION FUNCTIONS ### //

static struct Expression* alloc_literal(enum LiteralType type, void* value) {
  struct Expression* expr = malloc(sizeof(*expr));

  struct Literal literal = { .type = type };

  switch(type) {
  case LIT_BOOL:
    literal.as.boolean = *(bool*)value; break;
  case LIT_INT:
    literal.as.integer = *(size_t*)value; break;
  case LIT_FLOAT:
    literal.as.floating = *(double*)value; break;
  case LIT_CHAR:
    literal.as.character = *(char*)value; break;
  case LIT_STRING:
  case LIT_IDENTIFIER:
    literal.as.string = *(char**)value; break;
  }

  expr->type = EXPR_LITERAL;
  expr->as.literal = literal;

  return expr;
}

static struct Expression* alloc_unary(enum TokenType op,
    struct Expression* operand) {
  struct Expression* expr = malloc(sizeof(*expr));

  struct Unary unary = { .op = op, .operand = operand };

  expr->type = EXPR_UNARY;
  expr->as.unary = unary;

  return expr;
}

static struct Expression* alloc_binary(enum TokenType op,
    struct Expression* left, struct Expression* right) {
  struct Expression* expr = malloc(sizeof(*expr));

  struct Binary binary = { .op = op, .left = left, .right = right };

  expr->type = EXPR_BINARY;
  expr->as.binary = binary;

  return expr;
}

#define ALLOC_LITERAL(tag, _type, value) \
  ({ _type a = value; alloc_literal(LIT_##tag, &a); })



// ### PARSING FUNCTIONS ## //

static struct Expression* parse_primary(struct Parser* parser) {
  if(MATCH_TOKEN(parser, TRUE) || MATCH_TOKEN(parser, FALSE))
    return ALLOC_LITERAL(BOOL,bool,parser->previous.type==TOKEN_TRUE?true:false);
  if(MATCH_TOKEN(parser, INT_LIT))
    return ALLOC_LITERAL(INT, size_t, parser->previous.as.integer);
  if(MATCH_TOKEN(parser, FLOAT_LIT))
    return ALLOC_LITERAL(FLOAT, double, parser->previous.as.floating);
  if(MATCH_TOKEN(parser, CHAR_LIT))
    return ALLOC_LITERAL(CHAR, char, parser->previous.as.character);
  if(MATCH_TOKEN(parser, STRING_LIT))
    return ALLOC_LITERAL(STRING, const char*, parser->previous.as.string);
  if(MATCH_TOKEN(parser, IDENTIFIER_LIT))
    return ALLOC_LITERAL(IDENTIFIER, const char*, parser->previous.as.string);
  if(MATCH_TOKEN(parser, ERROR))
    return RETURN_ERROR(parser, parser->previous.as.integer);
  return RETURN_ERROR(parser, ERROR_EXPECTED_EXPRESSION);
}

static struct Expression* parse_call(struct Parser* parser) {
  return parse_primary(parser);
}

static struct Expression* parse_unary(struct Parser* parser) {
  if(MATCH_TOKEN(parser, BIT_NOT) || MATCH_TOKEN(parser, LOGIC_NOT)
      || MATCH_TOKEN(parser, SUB) || MATCH_TOKEN(parser, BIT_AND)
      || MATCH_TOKEN(parser, MUL)) {
    return alloc_unary(parser->previous.type, parse_unary(parser));
  }

  return parse_call(parser);
}

#define DEFINE_BINARY(name, prev, condition) \
static struct Expression* parse_##name(struct Parser* parser) { \
  struct Expression* left = parse_##prev(parser); \
  while(condition) { \
    enum TokenType op = parser->previous.type; \
    struct Expression* right = parse_##prev(parser); \
    left = alloc_binary(op, left, right); \
  } \
  return left; \
}


DEFINE_BINARY(factor, unary,
    MATCH_TOKEN(parser, MUL) || MATCH_TOKEN(parser, MUL_WRAP)
    || MATCH_TOKEN(parser, DIV) || MATCH_TOKEN(parser, MOD))

DEFINE_BINARY(term, factor,
    MATCH_TOKEN(parser, ADD) || MATCH_TOKEN(parser, ADD_WRAP)
    || MATCH_TOKEN(parser, SUB) || MATCH_TOKEN(parser, SUB_WRAP))

DEFINE_BINARY(bitwise, term,
    MATCH_TOKEN(parser, BIT_AND) || MATCH_TOKEN(parser, BIT_OR)
    || MATCH_TOKEN(parser, BIT_XOR) || MATCH_TOKEN(parser, BIT_SHR)
    || MATCH_TOKEN(parser, BIT_SHL))

DEFINE_BINARY(compare, bitwise,
    MATCH_TOKEN(parser, LT) || MATCH_TOKEN(parser, GT)
    || MATCH_TOKEN(parser, LT_EQ) || MATCH_TOKEN(parser, GT_EQ))

DEFINE_BINARY(equal, compare,
    MATCH_TOKEN(parser, EQ) || MATCH_TOKEN(parser, NOT_EQ))

DEFINE_BINARY(logic_and, equal, MATCH_TOKEN(parser, LOGIC_AND))
DEFINE_BINARY(logic_or, logic_and, MATCH_TOKEN(parser, LOGIC_OR))

#undef DEFINE_BINARY

static struct Expression* parse_cast(struct Parser* parser) {
  return parse_logic_or(parser);
}

static struct Expression* parse_assign(struct Parser* parser) {
  return parse_cast(parser);
}

struct Expression* parse_expression(struct Parser* parser) {
  return parse_assign(parser);
}



// ### PRINTING FUNCTIONS ## //

static void print_literal(const struct Literal* ast, size_t indent) {
  PRINT_INDENT(indent);
  printf("LITERAL ");

  if(ast == NULL) { printf("NULL\n"); return; }

  switch(ast->type) {
  case LIT_BOOL:       printf("%s", ast->as.boolean? "true" : "false"); break;
  case LIT_INT:        printf("%zu", ast->as.integer);                  break;
  case LIT_FLOAT:      printf("%f", ast->as.floating);                  break;
  case LIT_CHAR:       printf("'%c'", ast->as.character);               break;
  case LIT_STRING:     printf("\"%s\"", ast->as.string);                break;
  case LIT_IDENTIFIER: printf("%s", ast->as.string);                    break;
  }
  printf("\n");
}

static void print_unary(const struct Unary* ast, size_t indent) {
  PRINT_INDENT(indent);
  printf("UNARY ");

  if(ast == NULL) { printf("NULL\n"); return; }

  printf("%s\n", token_strings[ast->op]);
  print_expression(ast->operand, indent + 1);
  printf("\n");
}

static void print_binary(const struct Binary* ast, size_t indent) {
  PRINT_INDENT(indent);
  printf("BINARY ");

  if(ast == NULL) { printf("NULL\n"); }

  printf("%s\n", token_strings[ast->op]);
  print_expression(ast->left, indent + 1);
  print_expression(ast->right, indent + 1);
}

void print_expression(const struct Expression* ast, size_t indent) {
  PRINT_INDENT(indent);

  if(ast == NULL) { printf("NULL\n"); return; }

  switch(ast->type) {
    case EXPR_LITERAL: return print_literal(&ast->as.literal, indent + 1);
    case EXPR_UNARY:   return print_unary(&ast->as.unary, indent + 1);
    case EXPR_BINARY:  return print_binary(&ast->as.binary, indent + 1);
  }
}
