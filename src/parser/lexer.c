// lexer.c

#include "lexer.h"
#include <stdio.h>

static inline char next(const char** current) {
  *current += 1;
  return *(*current- 1);
}

static inline bool match(const char** current, char expected) {
  if(**current == '\0' || **current != expected) return false;
  *current += 1;
  return true;
}

static inline enum TokenType match_assign(const char** current,
    enum TokenType start_type) {
  return match(current, '=') ? start_type + 1 : start_type;
}

static inline enum TokenType match_wrap(const char** current,
    char e1, char e2, enum TokenType start_type) {
  return match(current, e1) ? start_type + 1 :
    match(current, e2) ? match(current, e1) ?
    start_type + 3 : start_type + 2 : start_type;
}

static inline enum TokenType match_arrow(const char** current,
    enum TokenType t1, enum TokenType t2) {
  return match(current, '>') ? t2 : match_wrap(current, '=', '%', t1);
}

static inline void skip_whitespace(const char** current) {
  while(true) {
    switch(**current) {
      case ' ':
      case '\r':
      case '\n':
      case '\t': next(current); break;
      case '/':
      default: return;
    }
  }
}

struct Token lex_token(const char** current) {
  const char* start = *current;

  skip_whitespace(current);
  if(**current == '\0') return TOKEN_NEW(TOKEN_EOF);

  char c = next(current);
  switch(c) {
    case '(': return TOKEN_NEW(TOKEN_LEFT_PAREN);
    case ')': return TOKEN_NEW(TOKEN_RIGHT_PAREN);
    case '[': return TOKEN_NEW(TOKEN_LEFT_BRACKET);
    case ']': return TOKEN_NEW(TOKEN_RIGHT_BRACKET);
    case '{': return TOKEN_NEW(TOKEN_LEFT_CURLY);
    case '}': return TOKEN_NEW(TOKEN_RIGHT_CURLY);
    case ';': return TOKEN_NEW(TOKEN_SEMICOLON);
    case ':': return TOKEN_NEW(TOKEN_COLON);
    case '.': return TOKEN_NEW(TOKEN_DOT);
    case ',': return TOKEN_NEW(TOKEN_COMMA);
    case '#': return TOKEN_NEW(TOKEN_HASHTAG);
    case '+': return TOKEN_NEW(match_wrap(current, '=', '%', TOKEN_ADD));
    case '-': return TOKEN_NEW(match_arrow(current, TOKEN_SUB, TOKEN_ARROW));
    case '*': return TOKEN_NEW(match_wrap(current, '=', '%', TOKEN_MUL));
    case '/': return TOKEN_NEW(match_assign(current, TOKEN_DIV));
    case '%': return TOKEN_NEW(match_assign(current, TOKEN_MOD));
    case '<': return TOKEN_NEW(match_wrap(current, '=', '<', TOKEN_LT));
    case '>': return TOKEN_NEW(match_wrap(current, '=', '>', TOKEN_GT));
    case '&': return TOKEN_NEW(match_assign(current, TOKEN_BIT_AND));
    case '|': return TOKEN_NEW(match_assign(current, TOKEN_BIT_OR));
    case '^': return TOKEN_NEW(match_assign(current, TOKEN_BIT_XOR));
    case '!': return TOKEN_NEW(match_assign(current, TOKEN_BIT_NOT));
    case '=': return TOKEN_NEW(match_arrow(current, TOKEN_EQ, TOKEN_EQ_ARROW));
  }

  return TOKEN_NEW_ERROR(&c);
}
