// declaration.c

#include <stdio.h>
#include "declaration.h"
#include "expression.h"

struct Variable* parse_variable(struct Parser* parser) {
  struct Variable* variable = malloc(sizeof(*variable));

  variable->vars = parse_vardecls(parser);

  EXPECT_TOKEN(parser, SEMICOLON, EXPECTED_END_OF_VARIABLE);

  return variable;
}


struct Struct* parse_struct(struct Parser* parser) {
  struct Struct* _struct = malloc(sizeof(*_struct));

  if(MATCH_TOKEN(parser, IDENTIFIER_LIT))
    _struct->name = parser->previous.as.string;
  else _struct->name = NULL;

  if(MATCH_TOKEN(parser, LEFT_CURLY)) {
    _struct->fields = parse_vardecls(parser);
    EXPECT_TOKEN(parser, RIGHT_CURLY, EXPECTED_END_OF_DECLARATION);
  } else _struct->fields = NULL;

  return _struct;
}


struct Union* parse_union(struct Parser* parser) {
  struct Union* _union = malloc(sizeof(*_union));

  if(MATCH_TOKEN(parser, IDENTIFIER_LIT))
    _union->name = parser->previous.as.string;
  else _union->name = NULL;

  if(MATCH_TOKEN(parser, LEFT_CURLY)) {
    _union->fields = parse_types(parser);
    EXPECT_TOKEN(parser, RIGHT_CURLY, EXPECTED_END_OF_DECLARATION);
  } else _union->fields = NULL;

  return _union;
}


struct Enum* parse_enum(struct Parser* parser) {
  struct Enum* _enum = malloc(sizeof(*_enum));

  if(MATCH_TOKEN(parser, IDENTIFIER_LIT))
    _enum->name = parser->previous.as.string;
  else _enum->name = NULL;

  if(MATCH_TOKEN(parser, LEFT_CURLY)) {
    _enum->fields = parse_assigns(parser);
    EXPECT_TOKEN(parser, RIGHT_CURLY, EXPECTED_END_OF_DECLARATION);
  } else _enum->fields = NULL;

  return _enum;
}


struct FuncSig* parse_funcsig(struct Parser* parser) {
  struct FuncSig* fs = malloc(sizeof(*fs));

  if(MATCH_TOKEN(parser, IDENTIFIER_LIT))
    fs->name = parser->previous.as.string;
  else fs->name = NULL;

  EXPECT_TOKEN(parser, LEFT_PAREN, EXPECTED_LEFT_PAREN);

  if(MATCH_TOKEN(parser, VOID))
    fs->args = NULL;
  else fs->args = parse_vardecls(parser);

  EXPECT_TOKEN(parser, RIGHT_PAREN, EXPECTED_RIGHT_PAREN);

  fs->returns = parse_types(parser);

  return fs;
}


static struct Function* parse_function(struct Parser* parser) {
  struct Function* func = malloc(sizeof(*func));

  func->sig = parse_funcsig(parser);

  EXPECT_TOKEN(parser, LEFT_CURLY, EXPECTED_BLOCK);

  func->body = parse_block(parser);

  return func;
}


// probably overkill lol
static const char* parse_include(struct Parser* parser) {
  EXPECT_TOKEN(parser, STRING_LIT, EXPECTED_STRING);
  const char* include = parser->previous.as.string;
  EXPECT_TOKEN(parser, SEMICOLON, EXPECTED_END_OF_DECLARATION);

  return include;
}


struct Declaration* parse_declaration(struct Parser* parser) {
  struct Declaration* decl = malloc(sizeof(*decl));

  if(MATCH_TOKEN(parser, LET)) {
    decl->type = DECL_VAR;
    decl->as.var = parse_variable(parser);

  } else if(MATCH_TOKEN(parser, STRUCT)) {
    decl->type = DECL_STRUCT;
    decl->as._struct = parse_struct(parser);

  } else if(MATCH_TOKEN(parser, UNION)) {
    decl->type = DECL_UNION;
    decl->as._union = parse_union(parser);

  } else if(MATCH_TOKEN(parser, ENUM)) {
    RETURN_ERROR(parser, ERROR_UNIMPLEMENTED);
    decl->type = DECL_ENUM;
    decl->as._enum = parse_enum(parser);

  } else if(MATCH_TOKEN(parser, FUNCTION)) {
    decl->type = DECL_FUNC;
    decl->as.function = parse_function(parser);

  } else if(MATCH_TOKEN(parser, INCLUDE)) {
    decl->type = DECL_INC;
    decl->as.include = parse_include(parser);

  } else RETURN_ERROR(parser, ERROR_EXPECTED_DECLARATION);


  return decl;
}



// ### PRINT FUNCTIONS ## //

void print_variable(const struct Variable* ast) {
  if(ast == NULL) { printf("(NULL)"); return; }

  print_vardecls(ast->vars);
}


void print_struct(const struct Struct* ast) {
  if(ast == NULL) { printf("(NULL)"); return; }

  printf("(struct ");
  if(ast->name) printf("%s ", ast->name);
  if(ast->fields) print_vardecls(ast->fields);
  printf(")");
}


void print_union(const struct Union* ast) {
  if(ast == NULL) { printf("(NULL)"); return; }

  printf("(union ");
  if(ast->name) printf("%s ", ast->name);
  if(ast->fields) print_types(ast->fields);
  printf(")");
}


void print_enum(const struct Enum* ast) {
  if(ast == NULL) { printf("(NULL)"); return; }

  printf("(enum ");
  if(ast->name) printf("%s ", ast->name);
  if(ast->fields) print_assigns(ast->fields);
  printf(")");
}


void print_funcsig(const struct FuncSig* ast) {
  if(ast == NULL) { printf("(NULL)"); return; }

  printf("(sig ");
  if(ast->name) printf("%s ", ast->name);

  printf("(");
  if(ast->args) { print_vardecls(ast->args); printf(") "); }
  else printf("void) ");

  printf("(");
  if(ast->returns) { print_types(ast->returns); printf(") "); }
  else printf("void) ");

  printf("\b)");
}


static void print_func(const struct Function* ast) {
  if(ast == NULL) { printf("(NULL)"); return; }

  printf("(function ");
  print_funcsig(ast->sig);
  printf(" (");
  print_block(ast->body);
  printf("))");
}


static void print_include(const char* ast) {
  if(ast == NULL) { printf("(NULL)"); return; }
  printf("(include \"%s\")", ast);
}


void print_declaration(const struct Declaration* ast) {
  if(ast == NULL) { printf("(NULL)"); return; }

  switch(ast->type) {
    case DECL_VAR:    print_variable(ast->as.var);    break;
    case DECL_STRUCT: print_struct(ast->as._struct);  break;
    case DECL_UNION:  print_union(ast->as._union);    break;
    case DECL_ENUM:   print_enum(ast->as._enum);      break;
    case DECL_FUNC:   print_func(ast->as.function);   break;
    case DECL_INC:    print_include(ast->as.include); break;
  }
}
