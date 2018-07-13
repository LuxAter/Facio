#include "lexer.h"

int main(int argc, char *argv[])
{
  facio_lexer lex;
  facio_lexer_init(&lex, "build");
  while(lex.tok.type != T_EOS){
    facio_scan(&lex);
    printf("[%s: %s]\n", facio_token_type_string(lex.tok.type), lex.tok.src);
  }
  return 0;
}
