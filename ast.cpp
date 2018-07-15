#include "ast.hpp"

#include <vector>

void Node::Push(const Node& child){
  children.push_back(child);
}
Node Node::Pop(){
  Node last = children.back();
  children.pop_back();
  return last;
}
