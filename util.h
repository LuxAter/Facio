#ifndef FACIO_UTIL_H_
#define FACIO_UTIL_H_

#include <stdio.h>

typedef struct vector {
  void** data;
  int size;
  int count;
} vector;

void vector_init(vector*);
int vector_count(vector*);
void vector_add(vector*, void*);
void vector_set(vector*, unsigned, void*);
void *vector_get(vector*, unsigned);
void vector_delete(vector*, unsigned);
void vector_free(vector*);

static inline int fpeekc(FILE* stream){
  int c;
  c = fgetc(stream);
  ungetc(c, stream);
  return c;
}

#endif  // FACIO_UTIL_H_
