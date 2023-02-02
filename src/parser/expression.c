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

static struct Expression* parse_group(struct Parser* parser) {
  struct Expression* group = malloc(sizeof(*group));

  struct Expression* expr = parse_expression(parser);

  EXPECT_TOKEN(parser, RIGHT_PAREN, EXPECTED_RIGHT_PAREN);

  group->type = EXPR_GROUP;

  if(group) group->as.group.expr = expr;
  return group;
}

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
  if(MATCH_TOKEN(parser, LEFT_PAREN))
    return parse_group(parser);
  if(MATCH_TOKEN(parser, ERROR))
    return RETURN_ERROR(parser, parser->previous.as.integer);
  if(MATCH_TOKEN(parser, EOF))
    return RETURN_ERROR(parser, ERROR_UNEXPECTED_EOF);
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

static struct Expression* parse_ifwhile(struct Parser* parser) {
  struct Expression* expr = malloc(sizeof(*expr));

  switch(parser->previous.type) {
    case TOKEN_IF: expr->type =    EXPR_IF;    break;
    case TOKEN_WHILE: expr->type = EXPR_WHILE; break;
    default: RETURN_ERROR(parser, ERROR_UNREACHABLE); return NULL;
  }

  struct IfWhile ifwhile;

  EXPECT_TOKEN(parser, LEFT_PAREN, EXPECTED_LEFT_PAREN);
  ifwhile.condition = parse_expression(parser);
  EXPECT_TOKEN(parser, RIGHT_PAREN, EXPECTED_RIGHT_PAREN);

  ifwhile.body = parse_expression(parser);

  if(MATCH_TOKEN(parser, ELSE))
    ifwhile.else_clause = parse_expression(parser);

  expr->as.ifwhile = ifwhile;
  return expr;
}

struct Expression* parse_expression(struct Parser* parser) {
  if(MATCH_TOKEN(parser, IF) || MATCH_TOKEN(parser, WHILE))
    return parse_ifwhile(parser);

  return parse_assign(parser);
}



// ### PRINTING FUNCTIONS ## //

static void print_literal(const struct Literal* ast) {
  printf("LITERAL ");

  if(ast == NULL) { printf("(NULL) "); return; }

  switch(ast->type) {
  case LIT_BOOL:       printf("%s ", ast->as.boolean? "true" : "false"); break;
  case LIT_INT:        printf("%zu ", ast->as.integer);                  break;
  case LIT_FLOAT:      printf("%f ", ast->as.floating);                  break;
  case LIT_CHAR:       printf("'%c' ", ast->as.character);               break;
  case LIT_STRING:     printf("\"%s\" ", ast->as.string);                break;
  case LIT_IDENTIFIER: printf("%s ", ast->as.string);                    break;
  }
}

static void print_unary(const struct Unary* ast) {
  printf("UNARY ");

  if(ast == NULL) { printf("(NULL) "); return; }

  printf("%s ", token_strings[ast->op]);
  print_expression(ast->operand);
}

static void print_binary(const struct Binary* ast) {
  printf("BINARY ");

  if(ast == NULL) { printf("(NULL) "); return; }

  printf("%s ", token_strings[ast->op]);
  print_expression(ast->left);
  print_expression(ast->right);
}

static void print_group(const struct Grouping* ast) {
  printf("GROUP ");

  if(ast == NULL) { printf("(NULL) "); return; }
  print_expression(ast->expr);
}

static void print_ifwhile(const struct Expression* ast) {
  if(ast->type == EXPR_IF) printf("IF ");
  else if(ast->type == EXPR_WHILE) printf("WHILE ");

  printf("(COND ");
  print_expression(ast->as.ifwhile.condition);
  printf(") ");

  printf("(BODY ");
  print_expression(ast->as.ifwhile.body);
  printf(") ");

  printf("(ELSE ");
  print_expression(ast->as.ifwhile.else_clause);
  printf(") ");
}

void print_expression(const struct Expression* ast) {
  if(ast == NULL) { printf("(NULL) "); return; }
  else printf("(");

  switch(ast->type) {
    case EXPR_LITERAL: print_literal(&ast->as.literal); break;
    case EXPR_UNARY:   print_unary(&ast->as.unary);     break;
    case EXPR_BINARY:  print_binary(&ast->as.binary);   break;
    case EXPR_GROUP:   print_group(&ast->as.group);     break;
    case EXPR_IF:      __attribute__((fallthrough));
    case EXPR_WHILE:   print_ifwhile(ast);              break;
  }
  printf(") ");
}
