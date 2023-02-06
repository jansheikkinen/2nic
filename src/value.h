#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

enum ValueType {
  VAL_UNDEFINED, VAL_BOOL, VAL_INT, VAL_FLOAT, VAL_CHAR,
  VAL_STRING, VAL_IDENTIFIER, VAL_PTR
};

struct Value {
  enum ValueType type;
  union {
    uintptr_t ptr;
    const char* string;
    size_t integer;
    double floating;
    char character;
    bool boolean;
  } as;
};

#define MATCH_VAL(val, _type) ((val)->type == VAL_##_type)
#define FROM_INT(val) ((val)->as.integer)

struct Value* alloc_int_val(size_t);
