// token.c

#include "token.h"

const char* token_strings[TOKEN_EOF] = {
  "TOKEN_UNDEFINED_TOKEN",
  "TOKEN_IDENTIFIER_LIT",
  "TOKEN_STRING_LIT",
  "TOKEN_UINT_LIT",
  "TOKEN_INT_LIT",
  "TOKEN_FLOAT_LIT",
  "TOKEN_CHAR_LIT",
  "TOKEN_BOOL_LIT",
  "TOKEN_ADD",
  "TOKEN_ADD_ASSIGN",
  "TOKEN_ADD_WRAP",
  "TOKEN_ADD_WRAP_ASSIGN",
  "TOKEN_SUB",
  "TOKEN_SUB_ASSIGN",
  "TOKEN_SUB_WRAP",
  "TOKEN_SUB_WRAP_ASSIGN",
  "TOKEN_MUL",
  "TOKEN_MUL_ASSIGN",
  "TOKEN_MUL_WRAP",
  "TOKEN_MUL_WRAP_ASSIGN",
  "TOKEN_DIV",
  "TOKEN_DIV_ASSIGN",
  "TOKEN_MOD",
  "TOKEN_MOD_ASSIGN",
  "TOKEN_LT",
  "TOKEN_LT_EQ",
  "TOKEN_BIT_SHL",
  "TOKEN_BIT_SHL_ASSIGN",
  "TOKEN_GT",
  "TOKEN_GT_EQ",
  "TOKEN_BIT_SHR",
  "TOKEN_BIT_SHR_ASSIGN",
  "TOKEN_BIT_AND",
  "TOKEN_BIT_AND_ASSIGN",
  "TOKEN_BIT_OR",
  "TOKEN_BIT_OR_ASSIGN",
  "TOKEN_BIT_XOR",
  "TOKEN_BIT_XOR_ASSIGN",
  "TOKEN_BIT_NOT",
  "TOKEN_NOT_EQ",
  "TOKEN_ASSIGN",
  "TOKEN_EQ",
  "TOKEN_LEFT_PAREN",
  "TOKEN_RIGHT_PAREN",
  "TOKEN_LEFT_BRACKET",
  "TOKEN_RIGHT_BRACKET",
  "TOKEN_LEFT_CURLY",
  "TOKEN_RIGHT_CURLY",
  "TOKEN_COLON",
  "TOKEN_SEMICOLON",
  "TOKEN_QUESTION",
  "TOKEN_DOT",
  "TOKEN_COMMA",
  "TOKEN_HASHTAG",
  "TOKEN_SMALL_ARROW",
  "TOKEN_LARGE_ARROW",
  "TOKEN_LOGIC_NOT",
  "TOKEN_LOGIC_AND",
  "TOKEN_LOGIC_OR",
  "TOKEN_LOGIC_XOR",
  "TOKEN_FUNCTION",
  "TOKEN_WHERE",
  "TOKEN_STRUCT",
  "TOKEN_ENUM",
  "TOKEN_UNION",
  "TOKEN_LET",
  "TOKEN_MUT",
  "TOKEN_UNDEFINED",
  "TOKEN_EXTERN",
  "TOKEN_INCLUDE",
  "TOKEN_IF",
  "TOKEN_ELSE",
  "TOKEN_WHILE",
  "TOKEN_FOR",
  "TOKEN_MATCH",
  "TOKEN_BREAK",
  "TOKEN_CONTINUE",
  "TOKEN_RETURN",
  "TOKEN_INT8",
  "TOKEN_INT16",
  "TOKEN_INT32",
  "TOKEN_INT64",
  "TOKEN_ISIZE",
  "TOKEN_UINT8",
  "TOKEN_UINT16",
  "TOKEN_UINT32",
  "TOKEN_UINT64",
  "TOKEN_USIZE",
  "TOKEN_FLOAT8",
  "TOKEN_FLOAT16",
  "TOKEN_FLOAT32",
  "TOKEN_FLOAT64",
  "TOKEN_FSIZE",
  "TOKEN_TYPE",
  "TOKEN_CHAR",
  "TOKEN_BOOL",
  "TOKEN_VOID",
  "TOKEN_NORETURN",
  "TOKEN_TRUE",
  "TOKEN_FALSE",
  "TOKEN_ERROR",
};
