#ifndef FACIO_TARGETS_H_
#define FACIO_TARGETS_H_

#include <stdint.h>
#include <string.h>

#include "ast.h"
#include "output.h"
#include "util.h"

typedef struct {
  const char* name;
  const char* description;
} target_t;

vector* get_targets(ast_node* ast);

#endif  // FACIO_TARGETS_H_
