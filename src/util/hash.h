#ifndef HASH_H
#define HASH_H

#include <stdint.h>
#include <stddef.h>

uint64_t hash(const uint8_t*);

struct HashItem {
  const uint8_t* key;
  size_t value;
};

struct HashMap {
  struct HashItem* values;
  size_t capacity;
  size_t length;
};

void hm_init(struct HashMap*);
void hm_destroy(struct HashMap*);

size_t hm_get(const struct HashMap*, const uint8_t*);

void hm_set(struct HashMap*, const uint8_t*, size_t);
void hm_remove(struct HashMap*, const uint8_t*);

#endif
