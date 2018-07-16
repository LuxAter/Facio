#ifndef FACIO_AST_H_
#define FACIO_AST_H_

#include <stdint.h>

#include "token.h"
#include "util.h"

enum NodeType {
  N_TRANSLATION_UNIT,
  N_INT,
  N_FLOAT,
  N_STRING,
  N_IDENTIFIER,
  N_BLOCK,
  N_UNARY_OP,
  N_BINARY_OP,
  N_IF_STMT,
  N_WHILE_STMT,
  N_RETURN_STMT,
  N_CALL_STMT,
  N_FUNCTION_DEF,
  N_TARGET_DEF
};

typedef struct ast_node {
  enum NodeType class;
  union {
    struct {
      long int value;
      facio_token_t tok;
    } int_;
    struct {
      double value;
      facio_token_t tok;
    } float_;
    struct {
      const char* value;
      facio_token_t tok;
    } string_;
    struct {
      const char* scope;
      const char* value;
    } identifier;
    struct {
      vector* stmts;
    } block;
    struct {
      facio_token_t op;
      struct ast_node* expr;
    } unary_op;
    struct {
      facio_token_t op;
      struct ast_node* left;
      struct ast_node* right;
    } binary_op;
    struct {
      struct ast_node* expr;
      struct ast_node* block;
      struct ast_node* else_block;
      vector* elif_blocks;
    } if_stmt;
    struct {
      struct ast_node* expr;
      struct ast_node* block;
    } while_stmt;
    struct {
      struct ast_node* expr;
    } return_stmt;
    struct {
      struct ast_node* id;
      vector* args;
    } call_stmt;
    struct {
      struct ast_node* name;
      struct ast_node* block;
      vector* params;
    } function_def;
    struct {
      const char* name;
      const char* description;
      vector* reqs;
      struct ast_node* block;
    } target_def;
    struct {
      vector* stmts;
    } translation_unit;
  };
} ast_node;

ast_node* int_node(facio_token_t tok);
ast_node* float_node(facio_token_t tok);
ast_node* string_node(facio_token_t tok);
ast_node* identifier_node(facio_token_t tok);
ast_node* scoped_identifier_node(facio_token_t scope, facio_token_t tok);
ast_node* block_node(vector* stmts);
ast_node* unary_op_node(facio_token_t tok, ast_node* expr);
ast_node* binary_op_node(facio_token_t tok, ast_node* left, ast_node* right);
ast_node* if_stmt_node(ast_node* expr, ast_node* stmts);
ast_node* while_stmt_node(ast_node* expr, ast_node* stmts);
ast_node* return_stmt_node(ast_node* expr);
ast_node* call_stmt_node(ast_node* func, vector* args);
ast_node* function_def_node(ast_node* name, vector* params, ast_node* stmts);
ast_node* target_def_node(facio_token_t tok, facio_token_t desc, vector* reqs, ast_node* stmts);
ast_node* translation_unit_node(vector* stmts);

#endif // FACIO_AST_H_
