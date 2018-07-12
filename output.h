#ifndef FACIO_OUTPUT_H_
#define FACIO_OUTPUT_H_

#include <stdlib.h>
#include <stdint.h>

enum Color{
  BLACK,
  RED,
  GREEN,
  YELLOW,
  BLUE,
  MAGENTA,
  CYAN,
  LIGHT_GRAY,
  DARK_GRAY,
  LIGHT_RED,
  LIGHT_GREEN,
  LIGHT_YELLOW,
  LIGHT_BLUE,
  LIGHT_MAGENTA,
  LIGHT_CYAN,
  WHITE,
  ABS_BLACK,
  ABS_WHITE,
  DEFAULT
};

char* clear(char* str);
char* bold(const char* str, char* dest, size_t size);
char* color(const char* str, enum Color fg, char* dest, size_t size);
char* color_rgb(const char* str, uint8_t r, uint8_t g, uint8_t b, char* dest, size_t size);

void fatal(const char* str, ...);
void error(const char* str, ...);
void warning(const char* str, ...);
void success(const char* str, ...);

#endif  // FACIO_OUTPUT_H_
