// parser.c

#include "../util/readfile.h"
#include "parser.h"

void parse_declaration(struct Parser*);

struct AST* parse_file(const char* filename) {
  struct Parser parser;
  parser.filename = filename;

  const char* program = read_file(filename);
  parser.program_index = program;


  printf("%24s | %s\n", "TOKEN TYPE", "LITERAL");
  struct Token token;
  while((token = lex_token(&parser.program_index)).type != TOKEN_EOF) {
    printf("%24s (%02d) | ", TOKEN_STR(&token), token.type);

    switch(token.type) {
      case TOKEN_IDENTIFIER_LIT:
      case TOKEN_STRING_LIT: printf("\"%s\"",  token.as.string);    break;
      case TOKEN_UINT_LIT:   printf("%zu",     token.as.uinteger);  break;
      case TOKEN_INT_LIT:    printf("%zd",     token.as.integer);   break;
      case TOKEN_FLOAT_LIT:  printf("%f",      token.as.floating);  break;
      case TOKEN_CHAR_LIT:   printf("'%c'",    token.as.character); break;
      case TOKEN_BOOL_LIT:   printf("%d",      token.as.boolean);   break;
      default: break;
    }
    printf("\n");
  }

  //while(parser.index < parser.tokens.size) parse_declaration(&parser);

  free((char*)program);

  return parser.ast;
}
