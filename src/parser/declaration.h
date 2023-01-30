#pragma once

struct Parser;

struct LValueList;
struct RvalueList;
struct EnumList;
struct TypeList;

struct Block;

enum DeclarationType {
  DECL_UNDEFINED,
  DECL_STRUCT,
  DECL_UNION,
  DECL_ENUM,
  DECL_FUNCTION,
  DECL_VARIABLE,
};

// "let" lvalue_list "=" rvalue_list
struct Variable {
  struct LValueList* lvalues;
  struct RValueList* rvalues;
};

// "struct" IDENTIFIER? "{" lvalue_list ";" "}"
struct Struct {
  const char* name;
  struct LValueList* lvalues;
};

// "union" IDENTIFIER? "{" lvalue_list ";" "}"
struct Union {
  const char* name;
  struct LValueList* lvalues;
};

// "enum" IDENTIFIER? "{" enum_list? "}"
struct Enum {
  const char* name;
  struct EnumList* enum_args;
};

// "function" IDENTIFIER "(" (lvalue_list | void) ")" type_list
//    [ "where" rvalue_list ] block
struct Function {
  const char* name;
  struct LValueList* lvalues;
  struct TypeList* types;
  struct RValueList* rvalues;
  struct Block* block;
};

// struct | enum | union | function | variable
struct Declaration {
  enum DeclarationType type;
  union {
    struct Struct _struct;
    struct Enum _enum;
    struct Union _union;
    struct Function function;
    struct Variable var;
  } as;
};

struct Declaration* parse_declaration(struct Parser*);
