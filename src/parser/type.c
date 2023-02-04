// type.c

#include "type.h"
#include <stdio.h>

struct Type* parse_type(struct Parser* parser) {
  struct Type* type = malloc(sizeof(*type));

  if(parser->current.type >= TOKEN_INT8 && parser->current.type < TOKEN_TRUE) {
    type->type = TYPE_PRIMITIVE;
    type->as.primitive = parser->current.type;

    parser->current = lex_token(parser);
    return type;
  }

  return type;
}


// ### PRINT FUNCTIONS ### //

void print_type(const struct Type* ast) {
  if(ast == NULL) { printf("(NULL)"); return; }

  switch(ast->type) {

    case TYPE_PRIMITIVE: printf("%s", token_strings[ast->as.primitive]); break;
    case TYPE_RESULT:
    case TYPE_OPTIONAL:
    case TYPE_REFERENCE:
    case TYPE_COMPOUND:
      break;
  }
}
