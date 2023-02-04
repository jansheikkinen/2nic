// declaration.c

#include "declaration.h"
#include <stdio.h>

struct Variable* parse_variable(struct Parser* parser) {
  struct Variable* variable = malloc(sizeof(*variable));

  variable->vars = parse_vardecls(parser);

  EXPECT_TOKEN(parser, SEMICOLON, EXPECTED_END_OF_VARIABLE);

  return variable;
}


// ### PRINT FUNCTIONS ## //

void print_variable(const struct Variable* ast) {
  if(ast == NULL) { printf("(NULL)"); return; }

  print_vardecls(ast->vars);
}
