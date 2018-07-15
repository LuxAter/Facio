#ifndef FACIO_AST_HPP_
#define FACIO_AST_HPP_

#include <vector>

struct Node {
  void Push(const Node& child);
  Node Pop();

  // Token tok;
  std::vector<Node> children;
};

#endif // FACIO_AST_HPP_
