// parser.c

#include "../util/readfile.h"
#include "../util/textcolor.h"
#include "declaration.h"
#include "expression.h"
#include "parser.h"

const char* error_strings[ERROR_FINAL] = {
  "unreachable",

  "unimplemented; check back later or add your own implementation",

  "unterminated string",
  "invalid character literal",
  "invalid symbol",

  "unexpected end of file",

  "expected declaration",
  "expected end of declaration; missing '}'?",
  "expected block",
  "expected end of block",
  "expected expression",
  "expected type",
  "expected end of statement",
  "expected end of variable; missing semicolon?",

  "expected an identifier",
  "expected '('",
  "expected ')'",
  "expected '{'",
  "expected '}'",
  "expected '['",
  "expected ']'",
  "expected assignment",
  "expected a string",
};

void print_error(struct Parser* ctx, enum ParseErrorType type) {
  ctx->is_panic = true; ctx->did_panic = true;

  set_color(COLATTR_BRIGHT, ERR_ERR_COLOR, COL_DEFAULT);
  printf("error");
  reset_color();

  printf(" @ ");
  set_color(COLATTR_BRIGHT, ERR_LOC_COLOR, COL_DEFAULT);
  printf("%s:(%zu, %zu)", ctx->filename, ctx->row, ctx->col);
  reset_color();

  if(ctx->previous.type >= TOKEN_IDENTIFIER_LIT
      && ctx->previous.type <= TOKEN_BOOL_LIT) {
    printf(" at literal ");
    switch(ctx->previous.type) {
      case TOKEN_IDENTIFIER_LIT:
      case TOKEN_STRING_LIT: printf("\"%s\"", ctx->previous.as.string);   break;
      case TOKEN_INT_LIT:    printf("%zd",    ctx->previous.as.integer);  break;
      case TOKEN_FLOAT_LIT:  printf("%f",     ctx->previous.as.floating); break;
      case TOKEN_CHAR_LIT:   printf("'%c'",   ctx->previous.as.character);break;
      case TOKEN_BOOL_LIT:   printf("%d",     ctx->previous.as.boolean);  break;
      default: break;
    }
    printf("\n");
  } else printf(" at token \"%s\"\n ", TOKEN_STR(&ctx->previous));

  set_color(COLATTR_BRIGHT, ERR_ERR_COLOR, COL_DEFAULT);
  printf("  %03d", type);
  reset_color();
  printf(": %s\n", error_strings[type]);
}

static void print_ast(const struct AST* ast) {
  for(size_t i = 0; i < ast->size; i++) {
    print_declaration(ast->members[i]);
    printf("\n");
  }
}

static void print_token(struct Parser* parser) {
  if(parser->current.type == TOKEN_EOF) return;

  printf("%24s %02u (%02zu, %02zu) | ",
    TOKEN_STR(&parser->current), parser->current.type,
    parser->row, parser->col);

  switch(parser->current.type) {
    case TOKEN_IDENTIFIER_LIT:
    case TOKEN_STRING_LIT: printf("\"%s\"",parser->current.as.string);   break;
    case TOKEN_INT_LIT:    printf("%zd",   parser->current.as.integer);  break;
    case TOKEN_FLOAT_LIT:  printf("%f",    parser->current.as.floating); break;
    case TOKEN_CHAR_LIT:   printf("'%c'",  parser->current.as.character);break;
    case TOKEN_BOOL_LIT:   printf("%d",    parser->current.as.boolean);  break;
    default: break;
  }
  printf("\n");
}

static void print_tokens(struct Parser* parser) {
  while((parser->current = lex_token(parser)).type != TOKEN_EOF)
    print_token(parser);
}

struct AST* parse_file(const char* filename, int flags) {
  struct Parser parser;
  parser.filename = filename;
  parser.col = 0; parser.row = 0;
  parser.is_panic = false;
  parser.flags = flags;

  const char* program = read_file(filename);
  parser.program_index = program;

  struct AST* ast = malloc(sizeof(*ast));
  NEW_ARRAYLIST(ast);

  if(HAS_FLAG(parser.flags, FLAG_LEX)) {
    print_tokens(&parser);
    parser.program_index = program;
  }

  parser.current = lex_token(&parser);
  while(!MATCH_TOKEN(&parser, EOF)) {
    APPEND_ARRAYLIST(ast, parse_declaration(&parser));
  }

  free((char*)program);

  if(HAS_FLAG(parser.flags, FLAG_AST)) print_ast(ast);

  if(!parser.did_panic) return ast;
  else return NULL;
}
