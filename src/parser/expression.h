#pragma once

#include "token.h"
#include <stddef.h>

struct Parser;
struct StatementList;

// "{" { statement ";" } expression? "}"
struct Block {
  struct StatementList* body;
  struct Expression* returns;
};

// if | while | for | block
struct Statement { };


// "(" expression ")" |  "{" rvalue_list? "}" | "undefined" | "true" | "false"
//  | NUMBER | STRING | CHARACTER
enum PrimaryType { PRIMARY_EXPRESSION, PRIMARY_RVALUE, PRIMARY_LITERAL };
struct Primary {
  enum PrimaryType type;
  union {
    struct Expression* expression;
    struct RValueList* rvalues;
    struct Token* literal;
  } as;
};

// primary { "(" rvalue_list? ")" | field | array_index }
enum CallType { CALL_RVALUE, CALL_FIELD, CALL_ARRAY_INDEX };
struct Call {
  enum CallType type;
  struct Primary* primary;
  union {
    struct RValueList* rvalues;
    struct Field* field;
    struct ArrayIndex* array_index;
  } as;
};

// ("not" | "!" | "-" | "*" | "&") unary | call
enum UnaryType { UNARY_UNARY, UNARY_CALL };
struct Unary {
  enum UnaryType type;
  enum TokenType op;
  union {
    struct Unary* unary;
    struct Call* call;
  } as;
};

// logic_and { "or" logic_and }
// equality { "and" equality }
// comparison { ("==" | "!=") comparison }
// bitwise { ("<" | "<=" | ">" | ">=") bitwise }
// term { ("&" | "|" | "^" | "<<" | ">>") term }
// factor { ("-" | "-%" | "+" | "+%") factor }
// unary { ("*" | "*%" | "/" | "%") unary }
struct Binary {
  enum TokenType op;
  struct Expression* left;
  struct Expression* right;
};

// assignment "as" type
struct Cast {
  struct Expression* expression;
  struct Type* type;
};

// cast | statement
struct Expression { };


// ("." | "->") IDENTIFIER
struct Field {
  const char* name;
  bool isptr;
};

// "[" expression "]"
struct ArrayIndex {
  struct Expression* expression;
};

struct Block* parse_block(struct Parser*);
