#include "parser.h"

#include <stdlib.h>

#include "ast.h"
#include "lexer.h"
#include "output.h"
#include "util.h"

void eat_token(facio_lexer* lexer, facio_token tok)
{
  facio_token_t t = lexer->tok;
  if (t.type != tok) {
    fatal("[PARSE ERROR] expected %s but got %s [%u:%u]", facio_token_type_string(tok), facio_token_type_string(t.type), t.line_num_, t.column_num_);
    exit(2);
  }
  facio_scan(lexer);
}

ast_node* facio_parse(facio_lexer* lexer)
{
  success("[TU]>>");
  facio_scan(lexer);
  ast_node* node = translation_unit_node(statement_list(lexer));
  success("[TU]<<");
  return node;
}
vector* statement_list(facio_lexer* lexer)
{
  success("[Statement List]>>");
  vector* vec = malloc(sizeof(vector));
  vector_init(vec);
  facio_token_t tok;
  while ((tok = facio_peek(lexer)).type != T_EOS && tok.type != T_DEDENT && lexer->tok.type != T_EOS && lexer->tok.type != T_DEDENT) {
    vector_add(vec, statement(lexer));
  }
  success("[Statement List]<<");
  return vec;
}
ast_node* statement(facio_lexer* lexer)
{
  success("[statement]>>");
  facio_token_t tok = lexer->tok;
  ast_node* node = NULL;
  if (tok.type == T_IF) {
    node = selection_statement(lexer);
  } else if (tok.type == T_WHILE || tok.type == T_FOR) {
    node = iteration_statement(lexer);
  } else if (tok.type == T_DEF) {
    node = function_definition(lexer);
  } else if (tok.type == T_IDENTIFIER) {
    if(facio_peek(lexer).type == T_SEMI_COLON){
      node = target_definition(lexer);
    }else{
      node = expression(lexer);
    }
  }else{
    node = expression(lexer);
  }
  if(node == NULL){
    fatal("[PARSE ERROR] expected [if, while, for, def, _id_] but got %s [%u:%u]", tok.src, tok.line_num_, tok.column_num_);
    exit(1);
  }
  if(facio_peek(lexer).type != T_EOS){
    eat_token(lexer, T_EOL);
  }
  return node;
}
ast_node* selection_statement(facio_lexer* lexer)
{
  success("[selection_statement]>>");
  eat_token(lexer, T_IF);
  eat_token(lexer, T_LPAREN);
  ast_node* expr = expression(lexer);
  eat_token(lexer, T_RPAREN);
  eat_token(lexer, T_EOL);
  ast_node* blk = block(lexer);
  ast_node* node = if_stmt_node(expr, blk);
  while (facio_peek(lexer).type == T_ELIF) {
    /* vector_add(node->if_stmt.elif_blocks, ) */
  }
  success("[selection_statement]<<");
  return node;
}
ast_node* iteration_statement(facio_lexer* lexer) {}
ast_node* expression(facio_lexer* lexer)
{
  return assignment_expression(lexer);
}
ast_node* assignment_expression(facio_lexer* lexer)
{
  ast_node* cond = conditional_expression(lexer);
  if (lexer->tok.type == T_OP_ASSIGN) {
    facio_token_t tok = lexer->tok;
    facio_scan(lexer);
    return binary_op_node(tok, cond, assignment_expression(lexer));
  } else {
    return cond;
  }
}
ast_node* conditional_expression(facio_lexer* lexer)
{
  ast_node* or_exp = logical_or_expression(lexer);
  if (lexer->tok.type == T_QMARK) {
    facio_scan(lexer);
    ast_node* node = if_stmt_node(or_exp, expression(lexer));
    node->if_stmt.else_block = conditional_expression(lexer);
    return node;
  }
  return or_exp;
}
ast_node* logical_or_expression(facio_lexer* lexer)
{
  ast_node* and_exp = logical_and_expression(lexer);
  if (lexer->tok.type == T_OP_OR) {
    facio_token_t tok = lexer->tok;
    facio_scan(lexer);
    return binary_op_node(tok, and_exp, logical_or_expression(lexer));
  }
  return and_exp;
}
ast_node* logical_and_expression(facio_lexer* lexer)
{
  ast_node* eq = equality_expression(lexer);
  if (lexer->tok.type == T_OP_AND) {
    facio_token_t tok = lexer->tok;
    facio_scan(lexer);
    return binary_op_node(tok, eq, logical_and_expression(lexer));
  }
  return eq;
}
ast_node* equality_expression(facio_lexer* lexer)
{
  ast_node* rela = relational_expression(lexer);
  if (lexer->tok.type == T_OP_EQ || lexer->tok.type == T_OP_NE) {
    facio_token_t tok = lexer->tok;
    facio_scan(lexer);
    return binary_op_node(tok, rela, equality_expression(lexer));
  }
  return rela;
}
ast_node* relational_expression(facio_lexer* lexer)
{
  ast_node* add = additive_expression(lexer);
  if (lexer->tok.type == T_OP_L || lexer->tok.type == T_OP_G || lexer->tok.type == T_OP_LE || lexer->tok.type == T_OP_GE) {
    facio_token_t tok = lexer->tok;
    facio_scan(lexer);
    return binary_op_node(tok, add, relational_expression(lexer));
  }
  return add;
}
ast_node* additive_expression(facio_lexer* lexer)
{
  ast_node* mul = multiplicative_expression(lexer);
  if (lexer->tok.type == T_OP_ADD || lexer->tok.type == T_OP_SUB) {
    facio_token_t tok = lexer->tok;
    facio_scan(lexer);
    return binary_op_node(tok, mul, additive_expression(lexer));
  }
  return mul;
}
ast_node* multiplicative_expression(facio_lexer* lexer)
{
  success("[multiplicative_expression]>>");
  ast_node* post = postfix_expression(lexer);
  if (lexer->tok.type == T_OP_MUL || lexer->tok.type == T_OP_DIV || lexer->tok.type == T_OP_MOD) {
    facio_token_t tok = lexer->tok;
    facio_scan(lexer);
    return binary_op_node(tok, post, additive_expression(lexer));
  }
  success("[multiplicative_expression]<<");
  return post;
}
ast_node* postfix_expression(facio_lexer* lexer)
{
  success("[postfix_expression]>>");
  ast_node* pri = primary_expression(lexer);
  if (lexer->tok.type == T_LPAREN) {
    eat_token(lexer, T_LPAREN);
    ast_node* call = call_stmt_node(pri, argument_list(lexer));
    eat_token(lexer, T_RPAREN);
    return call;
  }
  success("[postfix_expression]<<");
  return pri;
}
ast_node* primary_expression(facio_lexer* lexer)
{
  success("[primary_expression]>>");
  /* facio_scan(lexer); */
  ast_node* node = NULL;
  if (lexer->tok.type == T_INT) {
    printf("I::%li::\n", lexer->tok.value.int_);
    node = int_node(lexer->tok);
    facio_scan(lexer);
    return node;
  }
  if (lexer->tok.type == T_FLOAT) {
    printf("F::%f::\n", lexer->tok.value.float_);
    node = float_node(lexer->tok);
    facio_scan(lexer);
    return node;
  }
  if (lexer->tok.type == T_STRING) {
    printf("S::%s::\n", lexer->tok.value.string_);
    node = string_node(lexer->tok);
    facio_scan(lexer);
    return node;
  }
  if (lexer->tok.type == T_IDENTIFIER) {
    if (facio_peek(lexer).type == T_COLON) {
      facio_token_t scope = lexer->tok;
      eat_token(lexer, T_IDENTIFIER);
      eat_token(lexer, T_COLON);
      if (lexer->tok.type != T_IDENTIFIER) {
        fatal("[PARSE ERROR] expected [_id_] but got %s[%u:%u]", lexer->tok.src, lexer->tok.line_num_, lexer->tok.column_num_);
        exit(1);
      }
      printf("I::%s:%s::\n", scope.src, lexer->tok.src);
      node = scoped_identifier_node(scope, lexer->tok);
      facio_scan(lexer);
      return node;
    }
    printf("I::%s::\n", lexer->tok.src);
    node = identifier_node(lexer->tok);
    facio_scan(lexer);
    return node;
  }
  fatal("[PARSE ERROR] expected [_int_, _float_, _string_, _id_] but got %s [%u:%u]", lexer->tok.src, lexer->tok.line_num_, lexer->tok.column_num_);
  exit(1);
}
ast_node* function_definition(facio_lexer* lexer) {
  success("FUN>>");
  eat_token(lexer, T_DEF);
  ast_node* name = primary_expression(lexer);
  eat_token(lexer, T_LPAREN);
  vector* params = identifier_list(lexer);
  eat_token(lexer, T_RPAREN);
  eat_token(lexer, T_EOL);
  success("FUN<<");
  return function_def_node(name, params, block(lexer));
}
ast_node* target_definition(facio_lexer* lexer) {
  facio_token_t name = lexer->tok;
  facio_token_t desc = facio_get_token_string(T_STRING, "");
  if(facio_scan(lexer).type == T_STRING){
    desc = lexer->tok;
  }
  eat_token(lexer, T_SEMI_COLON);
  vector* req_list = identifier_list(lexer);
  return target_def_node(name, desc, req_list, block(lexer));
}
vector* identifier_list(facio_lexer* lexer) {
  vector* vec = malloc(sizeof(vector));
  vector_init(vec);
  if(facio_peek(lexer).type == T_RPAREN){
    facio_scan(lexer);
    return vec;
  }
  vector_add(vec, primary_expression(lexer));
  while(lexer->tok.type != T_RPAREN && lexer->tok.type != T_EOL){
    eat_token(lexer, T_COMMA);
    vector_add(vec, primary_expression(lexer));
  }
  facio_scan(lexer);
  return vec;
}
vector* argument_list(facio_lexer* lexer) {
  vector* vec = malloc(sizeof(vector));
  vector_init(vec);
  if(lexer->tok.type == T_RPAREN){
    return vec;
  }
  vector_add(vec, conditional_expression(lexer));
  while(lexer->tok.type != T_RPAREN){
    eat_token(lexer, T_COMMA);
    vector_add(vec, conditional_expression(lexer));
  }
  return vec;
}
ast_node* block(facio_lexer* lexer) {
  success("[block]>>");
  eat_token(lexer, T_INDENT);
  ast_node* node = block_node(statement_list(lexer));
  eat_token(lexer, T_DEDENT);
  success("[block]<<");
  return node;
}
