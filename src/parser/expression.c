// expression.c

#include "expression.h"
#include "declaration.h"
#include "list.h"
#include <stdio.h>


// ### ALLOCATION FUNCTIONS ### //

static struct Expression* alloc_literal(enum ValueType type, void* value) {
  struct Expression* expr = malloc(sizeof(*expr));

  struct Value literal = { .type = type };

  switch(type) {
  case VAL_BOOL:
    literal.as.boolean = *(bool*)value; break;
  case VAL_INT:
    literal.as.integer = *(size_t*)value; break;
  case VAL_FLOAT:
    literal.as.floating = *(double*)value; break;
  case VAL_CHAR:
    literal.as.character = *(char*)value; break;
  case VAL_STRING:
  case VAL_IDENTIFIER:
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
  ({ _type a = value; alloc_literal(VAL_##tag, &a); })



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


static struct Expression* parse_cast(struct Parser* parser) {
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

static struct Expression* parse_assign(struct Parser* parser) {
  struct Expression* left = parse_cast(parser);

  while(MATCH_ASSIGN_OPS(parser)) {
    enum TokenType op = parser->previous.type;
    struct Expression* right = parse_cast(parser);
    left = alloc_binary(op, left, right);
    left->type = EXPR_ASSIGN;
  }
  return left;
}

DEFINE_BINARY(fallback, assign,
    MATCH_TOKEN(parser, ORELSE) || MATCH_TOKEN(parser, CATCH))

#undef DEFINE_BINARY


static struct Expression* parse_block_expression(struct Parser*);

struct Block* parse_block(struct Parser* parser) {
  struct Block* block = malloc(sizeof(*block));

  NEW_ARRAYLIST(&block->stmts);
  block->expr = NULL;

  while(!MATCH_TOKEN(parser, RIGHT_CURLY) && !MATCH_TOKEN(parser, EOF)) {
    if(MATCH_TOKEN(parser, LET)) {
      struct Statement stmt;
      stmt.type = STMT_VAR;
      stmt.as.var = parse_variable(parser);
      APPEND_ARRAYLIST(&block->stmts, stmt);

    } else if(MATCH_TOKEN(parser, LEFT_CURLY)) {
      struct Block* blk = parse_block(parser);
      if(MATCH_TOKEN(parser, SEMICOLON)) {
        struct Expression* expr = malloc(sizeof(*expr));
        expr->type = EXPR_BLOCK;
        expr->as.block = *blk;
        free(blk);

        struct Statement stmt;
        stmt.type = STMT_EXPR;
        stmt.as.expr = expr;
        APPEND_ARRAYLIST(&block->stmts, stmt);
      } else {
        struct Statement stmt;
        stmt.type = STMT_BLOCK;
        stmt.as.block = blk;
        APPEND_ARRAYLIST(&block->stmts, stmt);
      }

    } else {
      struct Expression* expr = parse_expression(parser);
      if(MATCH_TOKEN(parser, SEMICOLON)) {
        parser->is_panic = false;

        struct Statement stmt;
        stmt.type = STMT_EXPR;
        stmt.as.expr = expr;
        APPEND_ARRAYLIST(&block->stmts, stmt);

      } else if(MATCH_TOKEN(parser, RIGHT_CURLY)) {
        block->expr = expr;
        break;

      } else RETURN_ERROR(parser, ERROR_EXPECTED_END_OF_BLOCK);
    }
  }

  return block;
}


static struct Expression* parse_block_expression(struct Parser* parser) {
  struct Expression* expr = malloc(sizeof(*expr));
  expr->type = EXPR_BLOCK;

  struct Block* block = parse_block(parser);
  expr->as.block = *block;
  free(block);

  return expr;
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


// this one is fun :) its just a vardecl + while loop in a block
static struct Expression* parse_for(struct Parser* parser) {
  EXPECT_TOKEN(parser, LEFT_PAREN, EXPECTED_LEFT_PAREN);

  struct Variable* init;
  if(MATCH_TOKEN(parser, SEMICOLON))
    init = NULL;
  else if(MATCH_TOKEN(parser, LET))
    init = parse_variable(parser);

  struct Expression* cond;
  if(MATCH_TOKEN(parser, SEMICOLON))
    cond = NULL;
  else {
    cond = parse_expression(parser);
    EXPECT_TOKEN(parser, SEMICOLON, EXPECTED_END_OF_STATEMENT);
  }

  struct Expression* inc;
  if(MATCH_TOKEN(parser, RIGHT_PAREN))
    inc = NULL;
  else {
    inc = parse_expression(parser);
    EXPECT_TOKEN(parser, RIGHT_PAREN, EXPECTED_RIGHT_PAREN);
  }

  struct Expression* body = parse_expression(parser);

  if(inc) {
    struct Statement stmt;
    stmt.type = STMT_EXPR;
    stmt.as.expr = body;

    struct Statement inc_stmt;
    inc_stmt.type = STMT_EXPR;
    inc_stmt.as.expr = inc;

    struct Expression* new_body = malloc(sizeof(*new_body));
    new_body->type = EXPR_BLOCK;
    NEW_ARRAYLIST(&new_body->as.block.stmts);

    APPEND_ARRAYLIST(&new_body->as.block.stmts, stmt);
    APPEND_ARRAYLIST(&new_body->as.block.stmts, inc_stmt);

    body = new_body;
  }

  if(!cond) cond = ALLOC_LITERAL(BOOL, bool, true);
  struct Expression* _while = malloc(sizeof(*_while));
  _while->type = EXPR_WHILE;
  _while->as.ifwhile = (struct IfWhile){
    .condition = cond,
    .body = body,
    .else_clause = NULL,
  };
  body = _while;

  if(init) {
    struct Statement init_stmt;
    init_stmt.type = STMT_VAR;
    init_stmt.as.var = init;

    struct Statement body_stmt;
    body_stmt.type = STMT_EXPR;
    body_stmt.as.expr = body;

    struct Expression* new_body = malloc(sizeof(*new_body));
    new_body->type = EXPR_BLOCK;
    NEW_ARRAYLIST(&new_body->as.block.stmts);
    APPEND_ARRAYLIST(&new_body->as.block.stmts, init_stmt);
    APPEND_ARRAYLIST(&new_body->as.block.stmts, body_stmt);

    body = new_body;
  }

  return body;
}


struct Expression* parse_expression(struct Parser* parser) {
  if(MATCH_TOKEN(parser, IF) || MATCH_TOKEN(parser, WHILE))
    return parse_ifwhile(parser);

  if(MATCH_TOKEN(parser, FOR))
    return parse_for(parser);

  if(MATCH_TOKEN(parser, LEFT_CURLY))
    return parse_block_expression(parser);

  if(MATCH_TOKEN(parser, CONTINUE) || MATCH_TOKEN(parser, BREAK)
      || MATCH_TOKEN(parser, RETURN)) {
    enum TokenType op = parser->previous.type;
    return alloc_unary(op, parse_expression(parser));
  }

  return parse_fallback(parser);
}



// ### PRINTING FUNCTIONS ## //

static void print_literal(const struct Value* ast) {
  if(ast == NULL) { printf("(NULL)"); return; }

  switch(ast->type) {
  case VAL_BOOL:       printf("%s", ast->as.boolean? "true" : "false"); break;
  case VAL_INT:        printf("%zu", ast->as.integer);                  break;
  case VAL_FLOAT:      printf("%f", ast->as.floating);                  break;
  case VAL_CHAR:       printf("'%c'", ast->as.character);               break;
  case VAL_STRING:     printf("\"%s\"", ast->as.string);                break;
  case VAL_IDENTIFIER: printf("%s", ast->as.string);                    break;
  }
}


static void print_unary(const struct Unary* ast) {
  if(ast == NULL) { printf("(NULL)"); return; }

  printf("%s ", token_strings[ast->op]);
  print_expression(ast->operand);
}


static void print_binary(const struct Binary* ast) {
  if(ast == NULL) { printf("(NULL)"); return; }

  printf("%s ", token_strings[ast->op]);
  print_expression(ast->left);
  printf(" ");
  print_expression(ast->right);
}


static void print_group(const struct Grouping* ast) {
  if(ast == NULL) { printf("(NULL)"); return; }
  printf("GROUP ");

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


static void print_statements(const struct StatementList*);

static void print_statement(const struct Statement* ast) {
  if(ast == NULL) { printf("(NULL)"); return; }

  switch(ast->type) {
    case STMT_EXPR:  print_expression(ast->as.expr); break;
    case STMT_BLOCK: print_block(ast->as.block);     break;
    case STMT_VAR:   print_variable(ast->as.var);    break;
  }
}


static void print_statements(const struct StatementList* ast) {
  if(ast == NULL) { printf("(NULL)"); return; }

  for(size_t i = 0; i < ast->size; i++) {
    print_statement(&ast->members[i]);
    printf(" ");
  }
  printf("\b");
}

void print_block(const struct Block* ast) {
  if(ast == NULL) { printf("(NULL)"); return; }

  printf("BLOCK (");
  print_statements(&ast->stmts);
  printf(")");

  if(ast->expr) {
    printf(" ");
    print_expression(ast->expr);
  }
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


void print_expression(const struct Expression* ast) {
  if(ast == NULL) { printf("(NULL)"); return; }

  if(ast->type != EXPR_LITERAL) printf("(");

  switch(ast->type) {
    case EXPR_LITERAL:     print_literal(&ast->as.literal);         break;
    case EXPR_UNARY:       print_unary(&ast->as.unary);             break;
    case EXPR_BINARY:      print_binary(&ast->as.binary);           break;
    case EXPR_GROUP:       print_group(&ast->as.group);             break;
    case EXPR_IF:          __attribute__((fallthrough));
    case EXPR_WHILE:       print_ifwhile(ast);                      break;
    case EXPR_BLOCK:       print_block(&ast->as.block);             break;
    case EXPR_CALL:        print_call(&ast->as.call);               break;
    case EXPR_FIELD:       print_field(&ast->as.field);             break;
    case EXPR_ARRAY_INDEX: print_array_index(&ast->as.array_index); break;
    case EXPR_ARRAY_INIT:  print_array_init(&ast->as.array_init);   break;
    case EXPR_LIST:        print_expressions(&ast->as.list);        break;
    case EXPR_CAST:        print_cast(&ast->as.cast);               break;
    case EXPR_ASSIGN:      print_binary(&ast->as.binary);           break;
    break;
  }

  if(ast->type != EXPR_LITERAL) printf(")");
}
