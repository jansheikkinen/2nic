// type.c

#include <stdio.h>
#include "type.h"
#include "expression.h"
#include "declaration.h"


struct Type* parse_type(struct Parser* parser) {
  struct Type* type = malloc(sizeof(*type));
  type->is_mutable = MATCH_TOKEN(parser, MUT);

  if(parser->current.type >= TOKEN_INT8 && parser->current.type < TOKEN_TRUE) {
    type->type = TYPE_PRIMITIVE;
    type->as.primitive = parser->current.type;

    // since i didn't use the iterator to get here
    parser->current = lex_token(parser);

  } else if(MATCH_TOKEN(parser, BIT_NOT) || MATCH_TOKEN(parser, QUESTION)
      || MATCH_TOKEN(parser, BIT_AND)) {
    type->type = TYPE_WRAPPER;
    type->as.wrapper.op = parser->previous.type;
    type->as.wrapper.type = parse_type(parser);

  } else if(MATCH_TOKEN(parser, LEFT_BRACKET)) {
    type->type = TYPE_ARRAY;

    if(!MATCH_TOKEN(parser, RIGHT_BRACKET)) {
      type->as.array.size = parse_expression(parser);
      EXPECT_TOKEN(parser, RIGHT_BRACKET, EXPECTED_RIGHT_BRACKET);
    }

    type->as.array.type = parse_type(parser);


  } else if(MATCH_TOKEN(parser, STRUCT)) {
    type->type = TYPE_COMPOUND;
    type->as.compound.type = COMP_STRUCT;
    type->as.compound.as._struct = parse_struct(parser);

  } else if(MATCH_TOKEN(parser, UNION)) {
    type->type = TYPE_COMPOUND;
    type->as.compound.type = COMP_UNION;
    type->as.compound.as._union = parse_union(parser);

  } else if(MATCH_TOKEN(parser, ENUM)) {
    type->type = TYPE_COMPOUND;
    type->as.compound.type = COMP_ENUM;
    type->as.compound.as._enum = parse_enum(parser);
  } else if(MATCH_TOKEN(parser, STRUCT)) {
    type->type = TYPE_COMPOUND;
    type->as.compound.type = COMP_FUNC;
    type->as.compound.as.sig = parse_funcsig(parser);
  }

  return type;
}


// ### PRINT FUNCTIONS ### //

static void print_primitive(const enum TokenType ast) {
  printf("%s", token_strings[ast]);
}


static void print_wrapper(const struct Wrapper* ast) {
  if(ast == NULL) { printf("(NULL)"); return; }

  printf("(%s ", token_strings[ast->op]);
  print_type(ast->type);
  printf(")");
}


static void print_array(const struct Array* ast) {
  if(ast == NULL) { printf("(NULL)"); return; }

  printf("([] ");
  print_expression(ast->size);
  printf(" ");
  print_type(ast->type);
  printf(")");
}


static void print_compound(const struct Compound* ast) {
  if(ast == NULL) { printf("(NULL)"); return; }

  printf("(");
  switch(ast->type) {
    case COMP_STRUCT: print_struct(ast->as._struct); break;
    case COMP_UNION:  print_union(ast->as._union);   break;
    case COMP_ENUM:   print_enum(ast->as._enum);     break;
    case COMP_FUNC:   print_funcsig(ast->as.sig);    break;
  }

}


void print_type(const struct Type* ast) {
  if(ast == NULL) { printf("(NULL)"); return; }

  if(ast->is_mutable) printf("mut ");
  switch(ast->type) {
    case TYPE_PRIMITIVE: print_primitive(ast->as.primitive); break;
    case TYPE_WRAPPER:   print_wrapper(&ast->as.wrapper);    break;
    case TYPE_ARRAY:     print_array(&ast->as.array);        break;
    case TYPE_COMPOUND:  print_compound(&ast->as.compound);  break;
  }
}
