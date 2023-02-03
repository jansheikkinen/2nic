// list.c

#include "list.h"
#include "expression.h"
#include <stdio.h>

struct ExprList* parse_exprs(struct Parser* parser);


struct VarDeclList* parse_vardecls(struct Parser* parser);


struct TypeList* parse_types(struct Parser* parser);



// ### PRINT FUNCTIONS ### //


void print_exprs(const struct ExprList*);


void print_vardelcs(const struct VarDeclList*);


void print_types(const struct TypeList*);
