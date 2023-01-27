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

static void skip_whitespace(const char** current) {
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
static struct Token lex_string(const char** current) {
  const char* start = *current;

  while(peek(current) != '"' && !is_at_end(current)) next(current);
  if(is_at_end(current)) return TOKEN_NEW_ERROR("unterminated string");

  char* literal = calloc(*current - start, sizeof(char));
  memcpy(literal, start, *current - start);

  next(current);
  return TOKEN_NEW_STRING(literal);
}

static struct Token lex_char(const char** current) {
  if(over(current) == '\'') {
    char literal = **current;
    next(current);
    next(current);

    return TOKEN_NEW_CHAR(literal);
  }

  return TOKEN_NEW_ERROR("invalid character literal");
}

static struct Token lex_number(const char** current) {
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

static struct Token check_keyword(const char* word, size_t start, size_t length,
    const char* rest, enum TokenType type) {
  if(strlen(word) == start + length && memcmp(word + start, rest, length) == 0) {
    free((char*)word);
    return TOKEN_NEW(type);
  }

  return TOKEN_NEW_IDENTIFIER(word);
}

static struct Token lex_identifier(const char** current) {
  const char* start = *current - 1;

  while(isalnum(peek(current)) || peek(current) == '_') next(current);

  size_t len = *current - start;
  char* literal = calloc(len, sizeof(char));
  memcpy(literal, start, len);

  switch(literal[0]) {
    case 'l': return check_keyword(literal, 1, 2, "et", TOKEN_LET);
    case 'n': return check_keyword(literal, 1, 7, "oreturn", TOKEN_NORETURN);
    case 'r': return check_keyword(literal, 1, 5, "eturn", TOKEN_RETURN);
    case 's': return check_keyword(literal, 1, 5, "truct", TOKEN_STRUCT);
    case 'v': return check_keyword(literal, 1, 3, "oid", TOKEN_VOID);
    case 'w': return check_keyword(literal, 1, 4, "hile", TOKEN_WHILE);
    case 'b':
      if(len > 1) { // bool break
        switch(literal[1]) {
          case 'o': return check_keyword(literal, 2, 2, "ol", TOKEN_BOOL);
          case 'r': return check_keyword(literal, 2, 3, "eak", TOKEN_BREAK);
        }
      } break;
    case 'c': // char continue
      if(len > 1) {
        switch(literal[1]) {
          case 'h': return check_keyword(literal, 2, 2, "ar", TOKEN_CHAR);
          case 'o': return check_keyword(literal, 2, 6, "ntinue", TOKEN_CONTINUE);
        }
      } break;
    case 'e': // else enum extern
      if(len > 1) {
        switch(literal[1]) {
          case 'l': return check_keyword(literal, 2, 2, "se", TOKEN_ELSE);
          case 'n': return check_keyword(literal, 2, 2, "um", TOKEN_ENUM);
          case 'x': return check_keyword(literal, 2, 4, "tern", TOKEN_EXTERN);
        }
      } break;
    case 'f': // false float for fsize function
      if(len > 1) {
        switch(literal[1]) {
          case 'a': return check_keyword(literal, 2, 3, "lse", TOKEN_FALSE);
          case 'l': break; // TODO: FLOAT TYPES
          case 'o': return check_keyword(literal, 2, 1, "r", TOKEN_FOR);
          case 's': return check_keyword(literal, 2, 3, "ize", TOKEN_FSIZE);
          case 'u': return check_keyword(literal, 2, 6, "nction", TOKEN_FUNCTION);
        }
      } break;
    case 'i': // if include int isize
      if(len > 1) {
        switch(literal[1]) {
          case 'f': free(literal); return TOKEN_NEW(TOKEN_IF);
          case 'n':
            if(len > 2) {
              switch(literal[2]) {
                case 'c':
                  return check_keyword(literal, 3, 4, "lude", TOKEN_INCLUDE);
                case 't':
                  if(len == 4)
                    return check_keyword(literal, 3, 1, "8", TOKEN_INT8);
                  if(len == 5) {
                    switch(literal[3]) {
                      case '1':
                        return check_keyword(literal, 4, 1, "6", TOKEN_INT16);
                      case '3':
                        return check_keyword(literal, 4, 1, "2", TOKEN_INT32);
                      case '6':
                        return check_keyword(literal, 4, 1, "4", TOKEN_INT64);
                    }
                  } break;
              }
            } break;
          case 's': return check_keyword(literal, 2, 3, "ize", TOKEN_ISIZE);
        }
      } break;
    case 'm': // match mut
      if(len > 1) {
        switch(literal[1]) {
          case 'a': return check_keyword(literal, 2, 3, "tch", TOKEN_MATCH);
          case 'u': return check_keyword(literal, 2, 1, "t", TOKEN_MUT);
        }
      } break;
    case 't': // true type
      if(len > 1) {
        switch(literal[1]) {
          case 'r': return check_keyword(literal, 2, 2, "ue", TOKEN_TRUE);
          case 'y': return check_keyword(literal, 2, 2, "pe", TOKEN_TYPE);
        }
      } break;
    case 'u': // uint union usize
      if(len > 1) {
        switch(literal[1]) {
          case 'i': break; // TODO: UINT
          case 'n': return check_keyword(literal, 2, 3, "ion", TOKEN_UNION);
          case 's': return check_keyword(literal, 2, 3, "ize", TOKEN_USIZE);
        }
      } break;
      break;
  }

  return TOKEN_NEW_IDENTIFIER(literal);
}

struct Token lex_token(const char** current) {
  skip_whitespace(current);
  if(**current == '\0') return TOKEN_NEW(TOKEN_EOF);

  char c = next(current);
  if(isdigit(c)) return lex_number(current);
  if(isalpha(c) || c == '_') return lex_identifier(current);

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
    case '"':  return lex_string(current);
    case '\'': return lex_char(current);
  }

  return TOKEN_NEW_ERROR(&c);
}
