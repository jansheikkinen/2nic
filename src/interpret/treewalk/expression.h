#pragma once

#include "ctx.h"
#include "../../parser/expression.h"

struct Value* walk_expression(struct Expression*, struct Interpreter*);
struct Value* walk_block(struct Block*, struct Interpreter*);
