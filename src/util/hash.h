#ifndef HASH_H
#define HASH_H

#include <stdint.h>
#include <stddef.h>

uint64_t hash(const uint8_t*);

struct HashItem {
  const char* key;
  uintptr_t value;
};

struct HashMap {
  struct HashItem* values;
  size_t capacity;
  size_t length;
};

void hm_init(struct HashMap*);
void hm_destroy(struct HashMap*);

uintptr_t hm_get(const struct HashMap*, const char*);

void hm_set(struct HashMap*, const char*, uintptr_t);
void hm_remove(struct HashMap*, const char*);

#endif
