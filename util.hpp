#ifndef FACIO_UTIL_HPP_
#define FACIO_UTIL_HPP_

#include <stdio.h>

static inline int fpeek(FILE* stream){
  int c;
  c = fgetc(stream);
  ungetc(c, stream);
  return c;
}

#endif  // FACIO_UTIL_HPP_
