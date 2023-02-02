// parser.c

#include "../util/readfile.h"
#include "../util/textcolor.h"
#include "expression.h"
#include "parser.h"

const char* error_strings[ERROR_FINAL] = {
  "unreachable",

  "unterminated string",
  "invalid character literal",
  "invalid symbol",

  "expected declaration",
  "expected expression",

  "expected an identifier",
  "expected '('",
  "expected ')'",
  "expected '{'",
  "expected '}'",
  "expected assignment",
};

void print_error(struct Parser* ctx, enum ParseErrorType type) {
  ctx->isPanic = true; ctx->didPanic = true;

  set_color(COLATTR_BRIGHT, ERR_ERR_COLOR, COL_DEFAULT);
  printf("error");
  reset_color();

  printf(" @ ");
  set_color(COLATTR_BRIGHT, ERR_LOC_COLOR, COL_DEFAULT);
  printf("%s:(%zu, %zu)", ctx->filename, ctx->row, ctx->col);
  reset_color();

  printf(" \"%s\"\n ", TOKEN_STR(&ctx->previous));

  set_color(COLATTR_BRIGHT, ERR_ERR_COLOR, COL_DEFAULT);
  printf("  %03d", type);
  reset_color();
  printf(": %s\n", error_strings[type]);
}

static void print_ast(const struct AST* ast) {
  printf("EXPRESSION:\n");
  for(size_t i = 0; i < ast->size; i++)
    print_expression(ast->members[i], 1);
}

struct AST* parse_file(const char* filename) {
  struct Parser parser;
  parser.filename = filename;
  parser.col = 0; parser.row = 0;
  parser.isPanic = false;

  const char* program = read_file(filename);
  parser.program_index = program;

  struct AST* ast = malloc(sizeof(*ast));
  NEW_ARRAYLIST(ast);

  // printf("%24s | %s\n", "TOKEN TYPE", "LITERAL");
  while((parser.current = lex_token(&parser)).type != TOKEN_EOF) {
    APPEND_ARRAYLIST(ast, parse_expression(&parser));

    // printf("%24s (%02zu, %02zu) | ", TOKEN_STR(&parser.token), parser.row, parser.col);
    //
    // switch(parser.token.type) {
    //   case TOKEN_IDENTIFIER_LIT:
    //   case TOKEN_STRING_LIT: printf("\"%s\"",  parser.token.as.string);    break;
    //   case TOKEN_INT_LIT:    printf("%zd",     parser.token.as.integer);   break;
    //   case TOKEN_FLOAT_LIT:  printf("%f",      parser.token.as.floating);  break;
    //   case TOKEN_CHAR_LIT:   printf("'%c'",    parser.token.as.character); break;
    //   case TOKEN_BOOL_LIT:   printf("%d",      parser.token.as.boolean);   break;
    //   default: break;
    // }
    // printf("\n");
  }

  if(!parser.didPanic)
    print_ast(ast);

  free((char*)program);

  return ast;
}
