// ctx.c

#include "ctx.h"
#include "../../util/panic.h"

void push_frame(struct Interpreter* ctx, const char* func) {
  struct Frame frame;
  frame.function = func;
  hm_init(&frame.variables);

  APPEND_ARRAYLIST(&ctx->frames, frame);
}

struct Frame* peek_frame(struct Interpreter* ctx, size_t index) {
  return &ctx->frames.members[index];
}

struct Frame* peek_current(struct Interpreter* ctx) {
  return peek_frame(ctx, ctx->frames.size - 1);
}

void pop_frame(struct Interpreter* ctx) {
  hm_destroy(&peek_current(ctx)->variables);
}

void set_variable(struct Interpreter* ctx, const char* name,
    struct Value* value) {
  hm_set(&peek_current(ctx)->variables, name, (uintptr_t)value);
}

struct Value* get_variable(struct Interpreter* ctx, const char* name) {
  for(size_t i = ctx->frames.size; i >= 0; i--) {
    struct Value* value =
      (struct Value*)hm_get(&peek_frame(ctx, i)->variables, name);
      if(value) return value;
  }

  panic(1, "no variable with that name");
  return NULL;
}
