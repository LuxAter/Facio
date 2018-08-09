#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include "targets.h"

int main(int argc, char *argv[])
{
  facio_lexer lex;
  facio_lexer_init(&lex, "build");
  ast_node* ast = facio_parse(&lex);
  print_ast(ast);
  vector* targets = get_targets(ast);
  for(int i = 0; i < vector_count(targets); ++i){
    target_t* t = (target_t*)vector_get(targets, i);
    printf("%s[%s]\n", t->name, t->description);
  }
  return 0;
}
