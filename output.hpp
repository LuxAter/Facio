#ifndef FACIO_OUTPUT_HPP_
#define FACIO_OUTPUT_HPP_

#include <array>
#include <string>

enum Color {
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

inline std::string Bold(const std::string& str) { return "\033[1m" + str + "\033[21m"; }
inline std::string Color(const std::string& str, Color fg)
{
  static std::array<std::string, 20> escape = { "\033[30m", "\033[31m", "\033[32m", "\033[33m", "\033[34m", "\033[35m", "\033[36m", "\033[37m", "\033[90m", "\033[91m", "\033[92m", "\033[93m", "\033[94m", "\033[95m", "\033[96m", "\033[97m", "\033[38;5;0m", "\033[38;5;256m", "\033[39m" };
  return escape.at(fg) + str + "\033[39m";
}
inline std::string Color(const std::string& str, uint8_t r, uint8_t g, uint8_t b)
{
  char escape[255];
  snprintf(escape, 255, "\033[38;2;%u;%u;%um", r, g, b);
  return std::string(escape) + str + "\033[39m";
}

inline void fatal(const std::string& str, ...)
{
  va_list args;
  va_start(args, str);
  vprintf(Color(Bold(str), LIGHT_RED).c_str(), args);
  printf("\n");
  va_end(args);
}
inline void error(const std::string& str, ...)
{
  va_list args;
  va_start(args, str);
  vprintf(Color(str, LIGHT_RED).c_str(), args);
  printf("\n");
  va_end(args);
}
inline void warning(const std::string& str, ...)
{
  va_list args;
  va_start(args, str);
  vprintf(Color(str, LIGHT_YELLOW).c_str(), args);
  printf("\n");
  va_end(args);
}
inline void success(const std::string& str, ...)
{
  va_list args;
  va_start(args, str);
  vprintf(Color(str, LIGHT_GREEN).c_str(), args);
  printf("\n");
  va_end(args);
}

#endif // FACIO_OUTPUT_HPP_
