#include "targets.h"

#include <stdint.h>
#include <string.h>

#include "ast.h"
#include "output.h"
#include "util.h"

vector* get_targets(ast_node* ast){
  if(ast->class != N_TRANSLATION_UNIT){
    fatal("[PARSE ERROR] Cannot scan for targets out of translation unit");
    exit(1);
  }
  vector* vec = malloc(sizeof(vector));
  vector_init(vec);
  for(int i = 0; i < vector_count(ast->translation_unit.stmts); ++i){
    ast_node* node = (ast_node*)vector_get(ast->translation_unit.stmts, i);
    if(node->class == N_TARGET_DEF){
      target_t* tar = (target_t*)malloc(sizeof(target_t));
      tar->name = node->target_def.name;
      tar->description = node->target_def.description;
      vector_add(vec, tar);
    }
  }
  return vec;
}
