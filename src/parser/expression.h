#pragma once

#include "../util/arraylist.h" // my greatest shame :(
#include "parser.h"

struct Statement {
  enum { STMT_EXPR, STMT_BLOCK, STMT_VAR } type;
  union {
    struct Block* block;
    struct Expression* expr;
    struct Variable* var;
  } as;
};

DEFINE_ARRAYLIST(StatementList, struct Statement);

struct Block {
  struct StatementList stmts;
  struct Expression* expr;
};

struct IfWhile {
  struct Expression* condition;
  struct Expression* body;
  struct Expression* else_clause;
};

enum LiteralType {
  LIT_BOOL, LIT_INT, LIT_FLOAT, LIT_CHAR, LIT_STRING, LIT_IDENTIFIER
};

struct Literal {
  enum LiteralType type;
  union {
    const char* string;
    size_t integer;
    double floating;
    char character;
    bool boolean;
  } as;
};

struct Unary {
  struct Expression* operand;
  enum TokenType op;
};

struct Binary {
  struct Expression* left;
  struct Expression* right;
  enum TokenType op;
};

struct Grouping {
  struct Expression* expr;
};

struct Call {
  struct Expression* callee;
  struct Expression* arguments;
};

struct Field {
  struct Expression* parent;
  const char* field;
};

struct ArrayIndex {
  struct Expression* array;
  struct Expression* index;
};

struct ArrayInit {
  struct Expression* elements;
};

struct ExprList {
  struct Expression* current;
  struct Expression* next;
};

struct Cast {
  struct Expression* expr;
  struct Type* type;
};

struct AssignList {
  struct Expression* current;
  struct Expression* next;
};

struct Expression {
  enum {
    EXPR_LITERAL, EXPR_UNARY, EXPR_BINARY, EXPR_GROUP, EXPR_CALL, EXPR_FIELD,
    EXPR_ARRAY_INDEX, EXPR_ARRAY_INIT, EXPR_CAST, EXPR_ASSIGN, EXPR_LIST,
    EXPR_BLOCK, EXPR_IF, EXPR_WHILE
  } type;
  union {
    // Statement Expressions
    struct Block block;
    struct IfWhile ifwhile;

    // Normal Expressions
    struct AssignList assign;
    struct Cast cast;
    struct ArrayIndex array_index;
    struct ArrayInit array_init;
    struct ExprList list;
    struct Field field;
    struct Call call;
    struct Grouping group;
    struct Binary binary;
    struct Unary unary;
    struct Literal literal;
  } as;
};

struct Expression* parse_expression(struct Parser*);
struct Block* parse_block(struct Parser*);

void print_expression(const struct Expression*);
