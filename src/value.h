#pragma once

#include <stdbool.h>
#include <stddef.h>

enum ValueType {
  VAL_BOOL, VAL_INT, VAL_FLOAT, VAL_CHAR, VAL_STRING, VAL_IDENTIFIER
};

struct Value {
  enum ValueType type;
  union {
    const char* string;
    size_t integer;
    double floating;
    char character;
    bool boolean;
  } as;
};
