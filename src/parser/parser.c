// parser.c

#include "../util/readfile.h"
#include "declaration.h"
#include "parser.h"

void append_error(struct Parser* ctx, enum ParseErrorType type) {
  ctx->isPanic = true;

  struct ParseError err = {
    .error_file = ctx->filename,
    .col = 0, .row = 0,
    .type = type
  };

  APPEND_ARRAYLIST(&ctx->errors, err);
}

struct AST* parse_file(const char* filename) {
  struct Parser parser;
  parser.filename = filename;

  const char* program = read_file(filename);
  parser.program_index = program;

  struct AST* ast = malloc(sizeof(*ast));
  NEW_ARRAYLIST(ast);

  // printf("%24s | %s\n", "TOKEN TYPE", "LITERAL");
  while((parser.token = lex_token(&parser)).type != TOKEN_EOF) {
    APPEND_ARRAYLIST(ast, parse_declaration(&parser));


    // printf("%24s (%02d) | ", TOKEN_STR(&token), token.type);
    //
    // switch(token.type) {
    //   case TOKEN_IDENTIFIER_LIT:
    //   case TOKEN_STRING_LIT: printf("\"%s\"",  token.as.string);    break;
    //   case TOKEN_UINT_LIT:   printf("%zu",     token.as.uinteger);  break;
    //   case TOKEN_INT_LIT:    printf("%zd",     token.as.integer);   break;
    //   case TOKEN_FLOAT_LIT:  printf("%f",      token.as.floating);  break;
    //   case TOKEN_CHAR_LIT:   printf("'%c'",    token.as.character); break;
    //   case TOKEN_BOOL_LIT:   printf("%d",      token.as.boolean);   break;
    //   default: break;
    // }
    // printf("\n");
  }


  free((char*)program);

  return ast;
}
