#ifndef FACIO_AST_H_
#define FACIO_AST_H_

#include <stdint.h>

#include "token.h"

typedef struct facio_node {
  facio_token_t tok;
  struct {
    uint8_t size_;
    struct facio_node** array;
  } children;
} facio_node;

void facio_init_ast(facio_node* self, facio_token_t tok);
void facio_push(facio_node* parent, facio_node* child);
void facio_pop(facio_node* parent, facio_node* child);
facio_node* facio_get(facio_node* parent, uint8_t i);

#endif // FACIO_AST_H_
