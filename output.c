#include "output.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* clear(char* str)
{
  str[0] = '\0';
  return str;
}

char* bold(const char* str, char* dest, size_t size)
{
  strncat(dest, "\033[1m", size);
  strncat(dest, str, size);
  strncat(dest, "\033[21m", size);
  return dest;
}

char* color(const char* str, enum Color fg, char* dest, size_t size)
{
  static char* fg_escape[] = { "\033[30m", "\033[31m", "\033[32m", "\033[33m", "\033[34m", "\033[35m", "\033[36m", "\033[37m", "\033[90m", "\033[91m", "\033[92m", "\033[93m", "\033[94m", "\033[95m", "\033[96m", "\033[97m", "\033[38;5;0m", "\033[38;5;256m", "\033[39m" };
  strncat(dest, fg_escape[fg], size);
  strncat(dest, str, size);
  strncat(dest, "\033[39m", size);
  return dest;
}

char* color_rgb(const char* str, uint8_t r, uint8_t g, uint8_t b, char* dest, size_t size)
{
  char escape[255];
  snprintf(escape, 255, "\033[38;2;%u;%u;%um", r, g, b);
  strncat(dest, escape, size);
  strncat(dest, str, size);
  strncat(dest, "\033[39m", size);
  return dest;
}

void fatal(const char* str, ...)
{
  char buf[255], tmp[255];
  clear(buf);
  clear(tmp);
  va_list args;
  va_start(args, str);
  vprintf(color(bold(str, buf, 255), LIGHT_RED, tmp, 255), args);
  printf("\n");
  va_end(args);
}
void error(const char* str, ...) {
  char buf[255];
  clear(buf);
  va_list args;
  va_start(args, str);
  vprintf(color(str, LIGHT_RED, buf, 255), args);
  printf("\n");
  va_end(args);
}
void warning(const char* str, ...) {

  char buf[255];
  clear(buf);
  va_list args;
  va_start(args, str);
  vprintf(color(str, LIGHT_YELLOW, buf, 255), args);
  printf("\n");
  va_end(args);
}
void success(const char* str, ...) {
  char buf[255];
  clear(buf);
  va_list args;
  va_start(args, str);
  vprintf(color(str, LIGHT_GREEN, buf, 255), args);
  printf("\n");
  va_end(args);
}
