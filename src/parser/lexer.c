// lexer.c

#include "lexer.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>

static inline char next(const char** current) {
  *current += 1;
  return *(*current - 1);
}

static inline bool match(const char** current, char expected) {
  if(**current == '\0' || **current != expected) return false;
  *current += 1;
  return true;
}

static inline bool is_at_end(const char** current) {
  return **current == '\0';
}

static inline char peek(const char** current) {
  return **current;
}

static inline char over(const char** current) {
  if(is_at_end(current)) return '\0';
  return (*current)[1];
}

static inline void skip_whitespace(const char** current) {
  while(true) {
    switch(**current) {
      case ' ':
      case '\r':
      case '\n':
      case '\t': next(current); break;
      case '/':
        if(over(current) == '/')
          while(peek(current) != '\n' && !is_at_end(current)) next(current);
        break;
      default: return;
    }
  }
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

// don't forget to free the string :)
static inline const char* match_string(const char** current) {
  const char* start = *current;

  while(peek(current) != '"' && !is_at_end(current)) next(current);
  if(is_at_end(current)) return "unterminated string";

  char* literal = calloc(*current - start, sizeof(char));
  memcpy(literal, start, *current - start);

  next(current);
  return literal;
}

static inline struct Token match_char(const char** current) {
  if(over(current) == '\'') {
    char literal = **current;
    next(current);
    next(current);

    return TOKEN_NEW_CHAR(literal);
  }

  return TOKEN_NEW_ERROR("invalid character literal");
}

static inline struct Token match_number(const char** current) {
  const char* start = *current - 1;
  bool is_floating = false;

  while(isdigit(peek(current))) next(current);
  if(peek(current) == '.' && isdigit(over(current))) {
    next(current); is_floating = true;
    while(isdigit(peek(current))) next(current);
  }

  char buf[*current - start];
  memset(buf, '\0', *current - start);
  memcpy(buf, start, *current - start);

  if(is_floating) return TOKEN_NEW_FLOAT(strtod(buf, NULL));
  else {
    long long literal = strtoll(buf, NULL, 0);
    if(literal < 0) return TOKEN_NEW_INT(literal);
    else return TOKEN_NEW_UINT(literal);
  }
}

struct Token lex_token(const char** current) {
  skip_whitespace(current);
  if(**current == '\0') return TOKEN_NEW(TOKEN_EOF);

  char c = next(current);
  if(isdigit(c)) return match_number(current);

  switch(c) {
    case '(':  return TOKEN_NEW(TOKEN_LEFT_PAREN);
    case ')':  return TOKEN_NEW(TOKEN_RIGHT_PAREN);
    case '[':  return TOKEN_NEW(TOKEN_LEFT_BRACKET);
    case ']':  return TOKEN_NEW(TOKEN_RIGHT_BRACKET);
    case '{':  return TOKEN_NEW(TOKEN_LEFT_CURLY);
    case '}':  return TOKEN_NEW(TOKEN_RIGHT_CURLY);
    case ';':  return TOKEN_NEW(TOKEN_SEMICOLON);
    case ':':  return TOKEN_NEW(TOKEN_COLON);
    case '.':  return TOKEN_NEW(TOKEN_DOT);
    case ',':  return TOKEN_NEW(TOKEN_COMMA);
    case '#':  return TOKEN_NEW(TOKEN_HASHTAG);
    case '+':  return TOKEN_NEW(match_wrap(current, '=', '%', TOKEN_ADD));
    case '-':  return TOKEN_NEW(match_arrow(current, TOKEN_SUB, TOKEN_ARROW));
    case '*':  return TOKEN_NEW(match_wrap(current, '=', '%', TOKEN_MUL));
    case '/':  return TOKEN_NEW(match_assign(current, TOKEN_DIV));
    case '%':  return TOKEN_NEW(match_assign(current, TOKEN_MOD));
    case '<':  return TOKEN_NEW(match_wrap(current, '=', '<', TOKEN_LT));
    case '>':  return TOKEN_NEW(match_wrap(current, '=', '>', TOKEN_GT));
    case '&':  return TOKEN_NEW(match_assign(current, TOKEN_BIT_AND));
    case '|':  return TOKEN_NEW(match_assign(current, TOKEN_BIT_OR));
    case '^':  return TOKEN_NEW(match_assign(current, TOKEN_BIT_XOR));
    case '!':  return TOKEN_NEW(match_assign(current, TOKEN_BIT_NOT));
    case '=':  return TOKEN_NEW(match_arrow(current, TOKEN_EQ, TOKEN_EQ_ARROW));
    case '"':  return TOKEN_NEW_STRING(match_string(current));
    case '\'': return match_char(current);
  }

  return TOKEN_NEW_ERROR(&c);
}
