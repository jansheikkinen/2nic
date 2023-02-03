// expression.c

#include "expression.h"
#include "list.h"
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


struct Expression* alloc_binary(enum TokenType op,
    struct Expression* left, struct Expression* right) {
  struct Expression* expr = malloc(sizeof(*expr));

  struct Binary binary = { .op = op, .left = left, .right = right };

  expr->type = EXPR_BINARY;
  expr->as.binary = binary;

  return expr;
}


static struct Expression* alloc_group(struct Expression* expr) {
  struct Expression* group = malloc(sizeof(*group));
  group->type = EXPR_GROUP;
  group->as.group.expr = expr;
  return group;
}


static struct Expression* alloc_call(struct Expression* callee,
    struct Expression* arguments) {
  struct Expression* expr = malloc(sizeof(*expr));
  expr->type = EXPR_CALL;
  expr->as.call.callee = callee;
  expr->as.call.arguments = arguments;
  return expr;
}


static struct Expression* alloc_field(struct Expression* parent,
    const char* field) {
  struct Expression* expr = malloc(sizeof(*expr));
  expr->type = EXPR_FIELD;
  expr->as.field.parent = parent;
  expr->as.field.field = field;
  return expr;
}


static struct Expression* alloc_array_index(struct Expression* array,
    struct Expression* index) {
  struct Expression* expr = malloc(sizeof(*expr));
  expr->type = EXPR_ARRAY_INDEX;
  expr->as.array_index.array = array;
  expr->as.array_index.index = index;
  return expr;
}


static struct Expression* alloc_cast(struct Expression* expr,
    struct Type* type) {
  struct Expression* cast = malloc(sizeof(*cast));
  cast->type = EXPR_CAST;
  cast->as.cast.expr = expr;
  cast->as.cast.type = type;
  return cast;
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


static struct Expression* parse_expressions(struct Parser* parser) {
  struct Expression* head = malloc(sizeof(*head));

  struct Expression* tail = head;
  tail->type = EXPR_LIST;
  tail->as.list.current = parse_expression(parser);

  while(MATCH_TOKEN(parser, COMMA)) {
    tail->as.list.next = malloc(sizeof(*(tail->as.list.next)));
    tail = tail->as.list.next;

    tail->type = EXPR_LIST;
    tail->as.list.current = parse_expression(parser);
  }

  // (a (b (c NULL))) -> (a (b c))
  struct Expression* c = tail->as.list.current;
  *tail = *c;
  free(c);

  return head;
}


static struct Expression* parse_array_init(struct Parser* parser) {
  struct Expression* expr = malloc(sizeof(*expr));
  expr->type = EXPR_ARRAY_INIT;

  expr->as.array_init.elements = parse_expressions(parser);

  EXPECT_TOKEN(parser, RIGHT_BRACKET, EXPECTED_RIGHT_BRACKET);

  return expr;
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
  if(MATCH_TOKEN(parser, LEFT_BRACKET))
    return parse_array_init(parser);
  if(MATCH_TOKEN(parser, ERROR))
    return RETURN_ERROR(parser, parser->previous.as.integer);
  if(MATCH_TOKEN(parser, EOF))
    return RETURN_ERROR(parser, ERROR_UNEXPECTED_EOF);
  return RETURN_ERROR(parser, ERROR_EXPECTED_EXPRESSION);
}


static struct Expression* parse_call(struct Parser* parser) {
  struct Expression* primary = parse_primary(parser);

  while(!MATCH_TOKEN(parser, EOF)) {
    if(MATCH_TOKEN(parser, LEFT_PAREN)) {
      if(MATCH_TOKEN(parser, RIGHT_PAREN))
        primary = alloc_call(primary, NULL);
      else {
        primary = alloc_call(primary, parse_expressions(parser));
        EXPECT_TOKEN(parser, RIGHT_PAREN, EXPECTED_RIGHT_PAREN);
      }

      // TODO: handle arguments; requires ExprList

    } else if(MATCH_TOKEN(parser, DOT)) {
      EXPECT_TOKEN(parser, IDENTIFIER_LIT, EXPECTED_IDENTIFIER);
      primary = alloc_field(primary, parser->previous.as.string);

    } else if(MATCH_TOKEN(parser, ARROW)) {
      EXPECT_TOKEN(parser, IDENTIFIER_LIT, EXPECTED_IDENTIFIER);
      primary = alloc_field(alloc_group(alloc_unary(TOKEN_MUL, primary)),
          parser->previous.as.string);

    } else if(MATCH_TOKEN(parser, LEFT_BRACKET)) {
      struct Expression* index = parse_expression(parser);
      EXPECT_TOKEN(parser, RIGHT_BRACKET, EXPECTED_RIGHT_BRACKET);
      primary = alloc_array_index(primary, index);

    } else break;
  }

  return primary;
}


static struct Expression* parse_unary(struct Parser* parser) {
  if(MATCH_TOKEN(parser, BIT_NOT) || MATCH_TOKEN(parser, LOGIC_NOT)
      || MATCH_TOKEN(parser, SUB) || MATCH_TOKEN(parser, BIT_AND)
      || MATCH_TOKEN(parser, MUL) || MATCH_TOKEN(parser, TRY)) {
    enum TokenType op = parser->previous.type;
    return alloc_unary(op, parse_unary(parser));
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


struct Expression* parse_cast(struct Parser* parser) {
  struct Expression* expression = parse_logic_or(parser);

  if(MATCH_TOKEN(parser, AS))
    expression = alloc_cast(expression, NULL); // TODO: cast type

  return expression;
}


#define MATCH_ASSIGN_OPS(parser) \
  MATCH_TOKEN(parser, ASSIGN) \
  || MATCH_TOKEN(parser, ADD_ASSIGN) \
  || MATCH_TOKEN(parser, SUB_ASSIGN) \
  || MATCH_TOKEN(parser, MUL_ASSIGN) \
  || MATCH_TOKEN(parser, ADD_WRAP_ASSIGN) \
  || MATCH_TOKEN(parser, SUB_WRAP_ASSIGN) \
  || MATCH_TOKEN(parser, MUL_WRAP_ASSIGN) \
  || MATCH_TOKEN(parser, DIV_ASSIGN) \
  || MATCH_TOKEN(parser, MOD_ASSIGN) \
  || MATCH_TOKEN(parser, BIT_SHR_ASSIGN) \
  || MATCH_TOKEN(parser, BIT_SHL_ASSIGN) \
  || MATCH_TOKEN(parser, BIT_AND_ASSIGN) \
  || MATCH_TOKEN(parser, BIT_XOR_ASSIGN) \
  || MATCH_TOKEN(parser, BIT_OR_ASSIGN)


DEFINE_BINARY(assign, cast, MATCH_ASSIGN_OPS(parser))


static struct Expression* parse_assigns(struct Parser* parser) {
  struct Expression* head = malloc(sizeof(*head));

  struct Expression* tail = head;
  tail->type = EXPR_ASSIGN;
  tail->as.assign.current = parse_assign(parser);

  while(MATCH_TOKEN(parser, COMMA)) {
    tail->as.assign.next = malloc(sizeof(*(tail->as.assign.next)));
    tail = tail->as.assign.next;

    tail->type = EXPR_ASSIGN;
    tail->as.assign.current = parse_assign(parser);
  }

  // (a (b (c NULL))) -> (a (b c))
  struct Expression* c = tail->as.assign.current;
  *tail = *c;
  free(c);

  return head;
}

DEFINE_BINARY(fallback, assigns,
    MATCH_TOKEN(parser, ORELSE) || MATCH_TOKEN(parser, CATCH))

#undef DEFINE_BINARY

static struct Expression* parse_block(struct Parser* parser) {
  struct Expression* head = malloc(sizeof(*head));

  struct Expression* tail = head;
  tail->type = EXPR_BLOCK;
  tail->as.block.expr = parse_expression(parser);

  while(MATCH_TOKEN(parser, SEMICOLON)) {
    tail->as.block.next = malloc(sizeof(*tail));
    tail = tail->as.block.next;

    tail->type = EXPR_BLOCK;
    tail->as.block.expr = parse_expression(parser);
  }

  EXPECT_TOKEN(parser, RIGHT_CURLY, EXPECTED_RIGHT_CURLY);

  // (a (b (c NULL))) -> (a (b c))
  struct Expression* c = tail->as.block.expr;
  *tail = *c;
  free(c);

  return head;
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
  if(MATCH_TOKEN(parser, LEFT_CURLY))
    return parse_block(parser);

  return parse_fallback(parser);
}



// ### PRINTING FUNCTIONS ## //

static void print_literal(const struct Literal* ast) {
  if(ast == NULL) { printf("(NULL)"); return; }

  switch(ast->type) {
  case LIT_BOOL:       printf("%s", ast->as.boolean? "true" : "false"); break;
  case LIT_INT:        printf("%zu", ast->as.integer);                  break;
  case LIT_FLOAT:      printf("%f", ast->as.floating);                  break;
  case LIT_CHAR:       printf("'%c'", ast->as.character);               break;
  case LIT_STRING:     printf("\"%s\"", ast->as.string);                break;
  case LIT_IDENTIFIER: printf("%s", ast->as.string);                    break;
  }
}


static void print_unary(const struct Unary* ast) {
  if(ast == NULL) { printf("(NULL) "); return; }

  printf("%s ", token_strings[ast->op]);
  print_expression(ast->operand);
}


static void print_binary(const struct Binary* ast) {
  if(ast == NULL) { printf("(NULL) "); return; }

  printf("%s ", token_strings[ast->op]);
  print_expression(ast->left);
  printf(" ");
  print_expression(ast->right);
}


static void print_group(const struct Grouping* ast) {
  printf("GROUP ");

  if(ast == NULL) { printf("(NULL)"); return; }
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
  printf(")");
}


static void print_block(const struct Expression* ast) {
  if(ast == NULL) { printf("(NULL) "); return; }

  printf("(");
  print_expression(ast->as.block.expr);
  printf(" ");

  print_expression(ast->as.block.next);

  printf(")");
}


static void print_expressions(const struct ExprList* ast) {
  if(ast == NULL) { printf("(NULL)"); return; }

  print_expression(ast->current);
  printf(" ");
  print_expression(ast->next);
}


static void print_call(const struct Call* ast) {
  if(ast == NULL) { printf("(NULL)"); return; }

  print_expression(ast->callee);

  printf(" (");
  if(ast->arguments)
    print_expression(ast->arguments);
  printf(")");
}


static void print_field(const struct Field* ast) {
  if(ast == NULL) { printf("(NULL)"); return; }

  printf(". ");
  print_expression(ast->parent);
  printf(" %s", ast->field);
}


static void print_array_index(const struct ArrayIndex* ast) {
  if(ast == NULL) { printf("(NULL)"); return; }

  printf("@ ");
  print_expression(ast->array);
  printf(" ");
  print_expression(ast->index);
}


static void print_array_init(const struct ArrayInit* ast) {
  if(ast == NULL) { printf("(NULL)"); return; }

  printf("[] ");
  print_expression(ast->elements);
}


static void print_cast(const struct Cast* ast) {
  if(ast == NULL) { printf("(NULL)"); return; }

  printf("as ");
  print_expression(ast->expr);

  printf(" (%%type");
  // TODO: type
  printf(")");
}


static void print_assigns(const struct AssignList* ast) {
  if(ast == NULL) { printf("(NULL)"); return; }

  print_expression(ast->current);
  printf(" ");
  print_expression(ast->next);
}


void print_expression(const struct Expression* ast) {
  if(ast == NULL) { printf("(NULL) "); return; }
  else printf("(");

  switch(ast->type) {
    case EXPR_LITERAL:     print_literal(&ast->as.literal);         break;
    case EXPR_UNARY:       print_unary(&ast->as.unary);             break;
    case EXPR_BINARY:      print_binary(&ast->as.binary);           break;
    case EXPR_GROUP:       print_group(&ast->as.group);             break;
    case EXPR_IF:          __attribute__((fallthrough));
    case EXPR_WHILE:       print_ifwhile(ast);                      break;
    case EXPR_BLOCK:       print_block(ast);                        break;
    case EXPR_CALL:        print_call(&ast->as.call);               break;
    case EXPR_FIELD:       print_field(&ast->as.field);             break;
    case EXPR_ARRAY_INDEX: print_array_index(&ast->as.array_index); break;
    case EXPR_ARRAY_INIT:  print_array_init(&ast->as.array_init);   break;
    case EXPR_LIST:        print_expressions(&ast->as.list);        break;
    case EXPR_CAST:        print_cast(&ast->as.cast);               break;
    case EXPR_ASSIGN:      print_assigns(&ast->as.assign);          break;
    break;
  }

  printf(")");
}
