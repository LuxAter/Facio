#include "parser.h"

#include <stdlib.h>

#include "ast.h"
#include "lexer.h"
#include "util.h"
#include "output.h"

void eat_token(facio_lexer* lexer, facio_token tok){
  facio_token_t t = facio_scan(lexer);
  if(t.type != tok){
    fatal("[PARSE ERROR] expected %s but got %s [%u:%u]", facio_token_type_string(tok), facio_token_type_string(t.type), t.line_num_, t.column_num_);
    exit(2);
  }
}

ast_node* facio_parse(facio_lexer* lexer)
{
  return translation_unit_node(statement_list(lexer));
}
vector* statement_list(facio_lexer* lexer)
{
  vector* vec = malloc(sizeof(vector));
  vector_init(vec);
  facio_token_t tok;
  while ((tok = facio_peek(lexer)).type != T_EOS && tok.type != T_DEDENT) {
    vector_add(vec, statement(lexer));
  }
  return vec;
}
ast_node* statement(facio_lexer* lexer) {
  facio_token_t tok = facio_scan(lexer);
  if(tok.type == T_IF){
    return selection_statement(lexer);
  }else if(tok.type == T_WHILE || tok.type == T_FOR){
    return iteration_statement(lexer);
  }else if(tok.type == T_DEF){
    return function_definition(lexer);
  }else if(tok.type == T_IDENTIFIER){
    return target_definition(lexer);
  }
  fatal("[PARSE ERROR] expected [if, while, for, def, _id_] but got %s [%u:%u]", tok.src, tok.line_num_, tok.column_num_);
  exit(1);
  return NULL;
}
ast_node* selection_statement(facio_lexer* lexer) {
  eat_token(lexer, T_LPAREN);
}
ast_node* iteration_statement(facio_lexer* lexer) {}
ast_node* expression(facio_lexer* lexer) {}
ast_node* assignment_expression(facio_lexer* lexer) {}
ast_node* conditional_expression(facio_lexer* lexer) {}
ast_node* logical_or_expression(facio_lexer* lexer) {}
ast_node* logical_and_expression(facio_lexer* lexer) {}
ast_node* equality_expression(facio_lexer* lexer) {}
ast_node* relational_expression(facio_lexer* lexer) {}
ast_node* additive_expression(facio_lexer* lexer) {}
ast_node* multiplicative_expression(facio_lexer* lexer) {}
ast_node* postfix_expression(facio_lexer* lexer) {}
ast_node* primary_expression(facio_lexer* lexer) {}
ast_node* function_definition(facio_lexer* lexer) {}
ast_node* target_definition(facio_lexer* lexer) {}
ast_node* identifier_list(facio_lexer* lexer) {}
ast_node* argument_list(facio_lexer* lexer) {}
ast_node* block(facio_lexer* lexer) {}
