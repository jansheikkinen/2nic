// hash.c

#include "hash.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FNV_OFFSET_BASIS ((uint64_t)14695981039346656037u)
#define FNV_PRIME ((uint64_t)1099511628211)

uint64_t hash(const uint8_t* bytes) {
  uint64_t hash = FNV_OFFSET_BASIS;

  for(const uint8_t* byte = bytes; *byte; byte++) {
    hash ^= (uint64_t)(*byte);
    hash *= FNV_PRIME;
  }

  return hash;
}

#undef FNV_OFFSET_BASIS
#undef FNV_PRIME


void hm_init(struct HashMap* hm) {
  hm->capacity = 16;
  hm->length = 0;

  hm->values = calloc(hm->capacity, sizeof(struct HashItem));
}
void hm_destroy(struct HashMap* hm) {
  for(size_t i = 0; i < hm->capacity; i++) free((void*)hm->values[i].key);
  free(hm->values);
}

static size_t hm_index(const struct HashMap* hm, const char* key) {
  return hash((uint8_t*)key) & (hm->capacity - 1);
}

static struct HashItem* _hm_get(const struct HashMap* hm, const char* key) {
  size_t index = hm_index(hm, key);
  struct HashItem* item = &hm->values[index];

  while((item = &hm->values[index++ & (hm->capacity - 1)])->key)
    if(!strcmp(item->key, key)) return item;

  return item;
}

uintptr_t hm_get(const struct HashMap* hm, const char* key) {
  struct HashItem* item = _hm_get(hm, key);
  if(item->key) return item->value;
  return 0;
}

static void _hm_set(struct HashMap* hm, const char* key, size_t value) {
  struct HashItem* item = _hm_get(hm, key);
  if(!item->key) {
    hm->length += 1;
    item->key = calloc(sizeof(*item->key), strlen(key) + 1);
    strcpy((char*)item->key, key);
  }
  item->value = value;
}

static void hm_expand(struct HashMap* hm) {
  hm->capacity <<= 1;
  struct HashItem* old = hm->values;
  hm->values = calloc(hm->capacity, sizeof(struct HashItem));

  for(size_t i = 0; i < (hm->capacity >> 1); i++) {
    struct HashItem* item = &old[i];
    if(item->key) _hm_set(hm, item->key, item->value);
  }

  free(old);
}

void hm_set(struct HashMap* hm, const char* key, size_t value) {
  if(hm->length >= (hm->capacity >> 1)) hm_expand(hm);
  _hm_set(hm, key, value);
}

void hm_remove(struct HashMap* hm, const char* key) {
  struct HashItem* item = _hm_get(hm, key);

  if(item->key) {
    free((char*)item->key);
    hm->length -= 1;
    item->key = NULL;
    item->value = 0;
  }
}
