#ifndef FACIO_PARSER_H_
#define FACIO_PARSER_H_

#include "lexer.h"
#include "ast.h"
#include "token.h"

void eat_token(facio_lexer* lexer, facio_token tok);

ast_node* facio_parse(facio_lexer* lexer);
vector* statement_list(facio_lexer* lexer);
ast_node* statement(facio_lexer* lexer);
ast_node* selection_statement(facio_lexer* lexer);
ast_node* iteration_statement(facio_lexer* lexer);
ast_node* expression(facio_lexer* lexer);
ast_node* assignment_expression(facio_lexer* lexer);
ast_node* conditional_expression(facio_lexer* lexer);
ast_node* logical_or_expression(facio_lexer* lexer);
ast_node* logical_and_expression(facio_lexer* lexer);
ast_node* equality_expression(facio_lexer* lexer);
ast_node* relational_expression(facio_lexer* lexer);
ast_node* additive_expression(facio_lexer* lexer);
ast_node* multiplicative_expression(facio_lexer* lexer);
ast_node* postfix_expression(facio_lexer* lexer);
ast_node* primary_expression(facio_lexer* lexer);
ast_node* function_definition(facio_lexer* lexer);
ast_node* target_definition(facio_lexer* lexer);
vector* identifier_list(facio_lexer* lexer);
vector* argument_list(facio_lexer* lexer);
ast_node* block(facio_lexer* lexer);


#endif  // FACIO_PARSER_H_
