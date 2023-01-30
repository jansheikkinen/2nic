#pragma once

struct Parser;

struct Type;

struct Expression;
struct Statement;

// IDENTIFIER [ ";" type ]
struct LValue {
  const char* name;
  struct Type* type;
};

#define NEW_LINKED_LIST(a, b) struct a { b type; struct a* list; }

// lvalue { "," lvalue }
NEW_LINKED_LIST(LValueList, struct LValue);

// expression { "," expression }
NEW_LINKED_LIST(RValueList, struct Expression*);

// IDENTIFIER "=" expression
struct EnumArg {
  const char* name;
  struct Expression* expression;
};

// enum_arg { "," enum_arg }
NEW_LINKED_LIST(EnumList, struct EnumArg);


// type { "," type }
NEW_LINKED_LIST(TypeList, struct Type*);


// statement ";" { statement ";" }
NEW_LINKED_LIST(StatementList, struct Statement*);

struct LValueList* parse_lvalues(struct Parser*);
struct RValueList* parse_rvalues(struct Parser*);
struct TypeList* parse_typelist(struct Parser*);
struct EnumList* parse_enumlist(struct Parser*);
