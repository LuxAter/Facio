#ifndef FACIO_UTIL_H_
#define FACIO_UTIL_H_

#include <stdio.h>

static inline int fpeek(FILE* stream){
  int c;
  c = fgetc(stream);
  ungetc(c, stream);
  return c;
}

#endif  // FACIO_UTIL_H_
