// value.c

#include "value.h"
#include <stdlib.h>

struct Value* alloc_int_val(size_t x) {
  struct Value* val = malloc(sizeof(*val));

  val->type = VAL_INT;
  val->as.integer = x;

  return val;
}
