#include "ast.h"

#include "token.h"

#include <stdlib.h>

ast_node* int_node(facio_token_t tok)
{
  ast_node* node = (ast_node*)malloc(sizeof(ast_node));
  node->class = N_INT;
  node->int_.value = tok.value.int_;
  node->int_.tok = tok;
  return node;
}
ast_node* float_node(facio_token_t tok)
{
  ast_node* node = (ast_node*)malloc(sizeof(ast_node));
  node->class = N_FLOAT;
  node->float_.value = tok.value.float_;
  node->float_.tok = tok;
  return node;
}
ast_node* string_node(facio_token_t tok)
{
  ast_node* node = (ast_node*)malloc(sizeof(ast_node));
  node->class = N_STRING;
  node->string_.value = tok.value.string_;
  node->string_.tok = tok;
  return node;
}
ast_node* identifier_node(facio_token_t tok)
{
  ast_node* node = (ast_node*)malloc(sizeof(ast_node));
  node->class = N_IDENTIFIER;
  node->identifier.scope = "L";
  node->identifier.value = tok.src;
  return node;
}
ast_node* scoped_identifier_node(facio_token_t scope, facio_token_t tok)
{
  ast_node* node = (ast_node*)malloc(sizeof(ast_node));
  node->class = N_IDENTIFIER;
  node->identifier.scope = scope.src;
  node->identifier.value = tok.src;
  return node;
}
ast_node* block_node(vector* stmts)
{
  ast_node* node = (ast_node*)malloc(sizeof(ast_node));
  node->class = N_BLOCK;
  node->block.stmts = stmts;
  return node;
}
ast_node* unary_op_node(facio_token_t tok, ast_node* expr)
{
  ast_node* node = (ast_node*)malloc(sizeof(ast_node));
  node->class = N_UNARY_OP;
  node->unary_op.op = tok;
  node->unary_op.expr = expr;
  return node;
}
ast_node* binary_op_node(facio_token_t tok, ast_node* left, ast_node* right)
{
  ast_node* node = (ast_node*)malloc(sizeof(ast_node));
  node->class = N_BINARY_OP;
  node->binary_op.op = tok;
  node->binary_op.left = left;
  node->binary_op.right = right;
  return node;
}
ast_node* if_stmt_node(ast_node* expr, ast_node* block)
{
  ast_node* node = (ast_node*)malloc(sizeof(ast_node));
  node->class = N_IF_STMT;
  node->if_stmt.expr = expr;
  node->if_stmt.block = block;
  node->if_stmt.else_block = NULL;
  vector_init(node->if_stmt.elif_blocks);
  return node;
}
ast_node* while_stmt_node(ast_node* expr, ast_node* block)
{
  ast_node* node = (ast_node*)malloc(sizeof(ast_node));
  node->class = N_WHILE_STMT;
  node->while_stmt.expr = expr;
  node->while_stmt.block = block;
  return node;
}
ast_node* return_stmt_node(ast_node* expr)
{
  ast_node* node = (ast_node*)malloc(sizeof(ast_node));
  node->class = N_RETURN_STMT;
  node->return_stmt.expr = expr;
  return node;
}
ast_node* call_stmt_node(ast_node* func, vector* args)
{
  ast_node* node = (ast_node*)malloc(sizeof(ast_node));
  node->class = N_CALL_STMT;
  node->call_stmt.id = func;
  node->call_stmt.args = args;
  return node;
}
ast_node* function_def_node(ast_node* name, vector* params, ast_node* stmts)
{
  ast_node* node = (ast_node*)malloc(sizeof(ast_node));
  node->class = N_FUNCTION_DEF;
  node->function_def.name = name;
  node->function_def.block = stmts;
  node->function_def.params = params;
  return node;
}
ast_node* target_def_node(facio_token_t tok, facio_token_t desc, vector* reqs, ast_node* stmts)
{

  ast_node* node = (ast_node*)malloc(sizeof(ast_node));
  node->class = N_TARGET_DEF;
  node->target_def.name = tok.value.string_;
  node->target_def.description = desc.value.string_;
  node->target_def.reqs = reqs;
  node->target_def.block = stmts;
  return node;
}
ast_node* translation_unit_node(vector* stmts){
  ast_node* node = (ast_node*)malloc(sizeof(ast_node));
  node->class = N_TRANSLATION_UNIT;
  node->translation_unit.stmts = stmts;
  return node;
}
