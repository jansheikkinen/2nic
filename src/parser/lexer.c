// lexer.c

#include "lexer.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define CURRENT(parser) ((parser)->program_index)

static inline char next(struct Parser* parser) {
  CURRENT(parser) += 1;
  parser->col += 1;
  return *(CURRENT(parser) - 1);
}

static inline bool match(struct Parser* parser, char expected) {
  if(*CURRENT(parser) == '\0' || *CURRENT(parser) != expected) return false;
  CURRENT(parser) += 1;
  return true;
}

static inline bool is_at_end(const struct Parser* parser) {
  return *CURRENT(parser) == '\0';
}

static inline char peek(const struct Parser* parser) {
  return *CURRENT(parser);
}

static inline char over(const struct Parser* parser) {
  if(is_at_end(parser)) return '\0';
  return *(CURRENT(parser) + 1);
}

static void skip_whitespace(struct Parser* parser) {
  while(true) {
    switch(*CURRENT(parser)) {
      case '\n': parser->row += 1; parser->col = 0;
                 __attribute__((fallthrough));
      case ' ':
      case '\r':
      case '\t': next(parser); break;
      case '/':
        if(over(parser) == '/')
          while(peek(parser) != '\n' && !is_at_end(parser)) next(parser);
        break;
      default: return;
    }
  }
}

static inline enum TokenType match_assign(struct Parser* parser,
    enum TokenType start_type) {
  return match(parser, '=') ? start_type + 1 : start_type;
}

static inline enum TokenType match_wrap(struct Parser* parser,
    char e1, char e2, enum TokenType start_type) {
  return match(parser, e1) ? start_type + 1 :
    match(parser, e2) ? match(parser, e1) ?
    start_type + 3 : start_type + 2 : start_type;
}

static inline enum TokenType match_arrow(struct Parser* parser,
    enum TokenType t1, enum TokenType t2) {
  return match(parser, '>') ? t2 : match_wrap(parser, '=', '%', t1);
}

// don't forget to free the string :)
static struct Token lex_string(struct Parser* parser) {
  const char* start = CURRENT(parser);

  while(peek(parser) != '"' && !is_at_end(parser)) next(parser);
  if(is_at_end(parser)) return TOKEN_NEW_ERROR(ERROR_LEX_UNTERMINATED_STRING);

  char* literal = calloc(CURRENT(parser) - start, sizeof(char));
  memcpy(literal, start, CURRENT(parser) - start);

  next(parser);
  return TOKEN_NEW_STRING(literal);
}

static struct Token lex_char(struct Parser* parser) {
  if(over(parser) == '\'') {
    char literal = *CURRENT(parser);
    next(parser);
    next(parser);

    return TOKEN_NEW_CHAR(literal);
  }

  return TOKEN_NEW_ERROR(ERROR_LEX_INVALID_CHAR_LITERAL);
}

static struct Token lex_number(struct Parser* parser) {
  const char* start = CURRENT(parser) - 1;
  bool is_floating = false;

  while(isdigit(peek(parser))) next(parser);
  if(peek(parser) == '.' && isdigit(over(parser))) {
    next(parser); is_floating = true;
    while(isdigit(peek(parser))) next(parser);
  }

  char buf[CURRENT(parser) - start];
  memset(buf, '\0', CURRENT(parser) - start);
  memcpy(buf, start, CURRENT(parser) - start);

  if(is_floating) return TOKEN_NEW_FLOAT(strtod(buf, NULL));
  else {
    long long literal = strtoll(buf, NULL, 0);
    return TOKEN_NEW_INT(literal);
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

static struct Token lex_identifier(struct Parser* parser) {
  const char* start = CURRENT(parser) - 1;

  while(isalnum(peek(parser)) || peek(parser) == '_') next(parser);

  size_t len = CURRENT(parser) - start;
  char* literal = calloc(len, sizeof(char));
  memcpy(literal, start, len);

  switch(literal[0]) {
    case 'l': return check_keyword(literal, 1, 2, "et", TOKEN_LET);
    case 'n': return check_keyword(literal, 1, 7, "oreturn", TOKEN_NORETURN);
    case 'o': return check_keyword(literal, 1, 1, "r", TOKEN_LOGIC_OR);
    case 'r': return check_keyword(literal, 1, 5, "eturn", TOKEN_RETURN);
    case 's': return check_keyword(literal, 1, 5, "truct", TOKEN_STRUCT);
    case 'v': return check_keyword(literal, 1, 3, "oid", TOKEN_VOID);
    case 'a':
      if(len > 1) {
        switch(literal[1]) {
          case 'n': return check_keyword(literal, 2, 1, "d", TOKEN_LOGIC_AND);
          case 's': return TOKEN_NEW(TOKEN_AS);
        }
      } break;
    case 'w':
      if(len > 2 && literal[1] == 'h') {
        switch(literal[2]) {
          case 'e': return check_keyword(literal, 3, 2, "re", TOKEN_WHERE);
          case 'i': return check_keyword(literal, 3, 2, "le", TOKEN_WHILE);
        }
      } break;
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
          case 'l':
            if(len >= 6 && literal[2] == 'o' && literal[3] == 'a'
                && literal[4] == 't') {
              if(len == 6 && literal[5] == '8') return TOKEN_NEW(TOKEN_FLOAT8);
              switch(literal[5]) {
                case '1': return check_keyword(literal, 6, 1, "6", TOKEN_FLOAT16);
                case '3': return check_keyword(literal, 6, 1, "2", TOKEN_FLOAT32);
                case '6': return check_keyword(literal, 6, 1, "4", TOKEN_FLOAT64);
              }
            } break;
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
          case 'i':
            if(len == 5 && literal[2] == 'n' && literal[3] == 't'
                && literal[4] == '8') {
              free(literal);
              return TOKEN_NEW(TOKEN_UINT8);
            }
            if(len > 5 && literal[2] == 'n' && literal[3] == 't') {
              switch(literal[4]) {
                case '1': return check_keyword(literal, 5, 1, "6", TOKEN_UINT16);
                case '3': return check_keyword(literal, 5, 1, "2", TOKEN_UINT32);
                case '6': return check_keyword(literal, 5, 1, "4", TOKEN_UINT64);
              }
            } break;
          case 'n': return check_keyword(literal, 2, 3, "ion", TOKEN_UNION);
          case 's': return check_keyword(literal, 2, 3, "ize", TOKEN_USIZE);
        }
      } break;
      break;
  }

  return TOKEN_NEW_IDENTIFIER(literal);
}

struct Token lex_token(struct Parser* parser) {
  skip_whitespace(parser);
  if(*CURRENT(parser) == '\0') return TOKEN_NEW(TOKEN_EOF);

  char c = next(parser);
  if(isdigit(c)) return lex_number(parser);
  if(isalpha(c) || c == '_') return lex_identifier(parser);

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
    case '+':  return TOKEN_NEW(match_wrap(parser, '=', '%', TOKEN_ADD));
    case '-':  return TOKEN_NEW(match_arrow(parser, TOKEN_SUB, TOKEN_ARROW));
    case '*':  return TOKEN_NEW(match_wrap(parser, '=', '%', TOKEN_MUL));
    case '/':  return TOKEN_NEW(match_assign(parser, TOKEN_DIV));
    case '%':  return TOKEN_NEW(match_assign(parser, TOKEN_MOD));
    case '<':  return TOKEN_NEW(match_wrap(parser, '=', '<', TOKEN_LT));
    case '>':  return TOKEN_NEW(match_wrap(parser, '=', '>', TOKEN_GT));
    case '&':  return TOKEN_NEW(match_assign(parser, TOKEN_BIT_AND));
    case '|':  return TOKEN_NEW(match_assign(parser, TOKEN_BIT_OR));
    case '^':  return TOKEN_NEW(match_assign(parser, TOKEN_BIT_XOR));
    case '!':  return TOKEN_NEW(match_assign(parser, TOKEN_BIT_NOT));
    case '=':  return TOKEN_NEW(match_arrow(parser, TOKEN_ASSIGN, TOKEN_EQ_ARROW));
    case '"':  return lex_string(parser);
    case '\'': return lex_char(parser);
  }

  return TOKEN_NEW_ERROR(ERROR_LEX_INVALID_SYMBOL);
}
