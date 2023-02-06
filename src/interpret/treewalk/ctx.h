#pragma once

#include "../../value.h"
#include "../../util/hash.h"
#include "../../util/arraylist.h"
#include "../../parser/type.h"

struct Var {
  struct Type* type;
  struct Value value;
};

struct Frame {
  const char* function;
  struct HashMap variables;
};

DEFINE_ARRAYLIST(StackFrames, struct Frame);

struct Interpreter {
  struct StackFrames frames;
};
