#include "lexer.h"
#include "parser.h"
#include "ast.h"

int main(int argc, char *argv[])
{
  facio_lexer lex;
  facio_lexer_init(&lex, "build");
  ast_node* ast = facio_parse(&lex);
  print_ast(ast);
  /* int i = 0; */
  /* while(lex.tok.type != T_EOS){ */
  /*   i++; */
  /*   if(i == 7){ */
  /*     facio_peek(&lex); */
  /*   } */
  /*   facio_scan(&lex); */
  /*   printf("[%s: %s]\n", facio_token_type_string(lex.tok.type), lex.tok.src); */
  /* } */
  return 0;
}
