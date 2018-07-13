#include "ast.h"

#include "token.h"

void facio_init_ast(facio_node* self, facio_token_t tok){
  self->tok = tok;
  self->children.size_ = 0;
}

void facio_push(facio_node* parent, facio_node* child){
  struct facio_node** new = realloc(parent->children.array, parent->children.size_ + 1);
  if(new){
    parent->children.array = new;
    memcpy(parent->children.array[parent->children.size_], child)
    parent->children.array[parent->children.size_] = child;
    parent->children.size_++;
  }
}
void facio_push
