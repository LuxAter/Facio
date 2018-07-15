#include "output.hpp"

#include <cstdio>
#include <string>

std::string Bold(const std::string& str){
  return "\033[1m" + str + "\033[21m";
}
