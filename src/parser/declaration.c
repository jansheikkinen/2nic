// declaration.c

#include "declaration.h"
#include "expression.h"
#include "lexer.h"
#include "list.h"
#include "parser.h"

// note: expects field and assigned value to be the same
#define ALLOC_DECL(field, _type) ({ \
    struct Declaration* decl = malloc(sizeof(*decl)); \
    if(decl) *decl = (struct Declaration){ .type=_type, .as={ .field=field }}; \
    decl; \
  })

static struct Declaration* parse_struct(struct Parser* parser) {
  struct Struct _struct;

  struct Token token;
  if(MATCH_TOKEN(parser, token, TOKEN_IDENTIFIER_LIT))
    _struct.name = token.as.string;

  EXPECT_TOKEN(parser, token, TOKEN_LEFT_CURLY, ERROR_EXPECTED_LEFT_CURLY);
  _struct.lvalues = parse_lvalues(parser);
  EXPECT_TOKEN(parser, token, TOKEN_RIGHT_CURLY, ERROR_EXPECTED_RIGHT_CURLY);

  return ALLOC_DECL(_struct, DECL_STRUCT);
}

static struct Declaration* parse_union(struct Parser* parser) {
  struct Union _union;

  struct Token token;
  if(MATCH_TOKEN(parser, token, TOKEN_IDENTIFIER_LIT))
    _union.name = token.as.string;

  EXPECT_TOKEN(parser, token, TOKEN_LEFT_CURLY, ERROR_EXPECTED_LEFT_CURLY);
  _union.lvalues = parse_lvalues(parser);
  EXPECT_TOKEN(parser, token, TOKEN_RIGHT_CURLY, ERROR_EXPECTED_RIGHT_CURLY);

  return ALLOC_DECL(_union, DECL_UNION);
}

static struct Declaration* parse_enum(struct Parser* parser) {
  struct Enum _enum;

  struct Token token;
  if(MATCH_TOKEN(parser, token, TOKEN_IDENTIFIER_LIT))
    _enum.name = token.as.string;

  EXPECT_TOKEN(parser, token, TOKEN_LEFT_CURLY, ERROR_EXPECTED_LEFT_CURLY);
  _enum.enum_args = parse_enumlist(parser);
  EXPECT_TOKEN(parser, token, TOKEN_RIGHT_CURLY, ERROR_EXPECTED_RIGHT_CURLY);

  return ALLOC_DECL(_enum, DECL_ENUM);

}

static struct Declaration* parse_function(struct Parser* parser) {
  struct Function function;

  struct Token token;
  EXPECT_TOKEN(parser, token, TOKEN_IDENTIFIER_LIT, ERROR_EXPECTED_IDENTIFIER);
  function.name = token.as.string;

  EXPECT_TOKEN(parser, token, TOKEN_LEFT_PAREN, ERROR_EXPECTED_LEFT_PAREN);
  function.lvalues = parse_lvalues(parser);
  EXPECT_TOKEN(parser, token, TOKEN_RIGHT_PAREN, ERROR_EXPECTED_RIGHT_PAREN);

  function.types = parse_typelist(parser);

  if(MATCH_TOKEN(parser, token, TOKEN_WHERE))
    function.rvalues = parse_rvalues(parser);

  function.block = parse_block(parser);

  return ALLOC_DECL(function, DECL_FUNCTION);
}

static struct Declaration* parse_variable(struct Parser* parser) {
  struct Variable var;

  var.lvalues = parse_lvalues(parser);

  struct Token token;
  EXPECT_TOKEN(parser, token, TOKEN_ASSIGN, ERROR_EXPECTED_ASSIGN);

  var.rvalues = parse_rvalues(parser);

  return ALLOC_DECL(var, DECL_VARIABLE);
}

struct Declaration* parse_declaration(struct Parser* parser) {
  switch(parser->token.type) {
    case TOKEN_STRUCT:   return parse_struct(parser);
    case TOKEN_UNION:    return parse_union(parser);
    case TOKEN_ENUM:     return parse_enum(parser);;
    case TOKEN_FUNCTION: return parse_function(parser);
    case TOKEN_LET:      return parse_variable(parser);
    default: return RETURN_ERROR(parser, ERROR_INVALID_DECLARATION);
  }
}
