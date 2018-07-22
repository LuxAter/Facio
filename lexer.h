#ifndef FACIO_LEXER_H_
#define FACIO_LEXER_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "token.h"

#define FACIO_STATE_LIST \
  s(TRANSLATION_UNIT, "translation-unit") \
  s(STATEMENT_LIST, "statement-list") \
  s(STATEMENT, "statement") \
  s(SELECTION_STATEMENT, "selection-statement") \
  s(SELECTION_STATEMENT_B, "selection-statement-b") \
  s(ITERATION_STATEMENT, "iteration-statement") \
  s(EXPRESSION, "expression") \
  s(ASSIGNMENT_EXPRESSION, "assignment-expression") \
  s(CONDITIONAL_EXPRESSION, "conditional-expression") \
  s(LOGICAL_OR_EXPRESSION, "logical-or-expression") \
  s(LOGICAL_AND_EXPRESSION, "logical-and-expression") \
  s(EQUALITY_EXPRESSION, "equality-expression") \
  s(RELATIONAL_EXPRESSION, "relational_expression") \
  s(ADDITIVE_EXPRESSION, "additive-expression") \
  s(MULTIPLICATIVE_EXPRESSION, "multiplicative-expression") \
  s(POSTFIX_EXPRESSION, "postfix-expression") \
  s(PRIMARY_EXPRESSION, "primary-expression") \
  s(FUNCTION_DEFINITION, "function-definition") \
  s(TARGET_DEFINITION, "target-definition") \
  s(IDENTIFIER_LIST, "identifier-list") \
  s(ARGUMENT_EXPRESSION_LIST, "argument-expression-list") \
  s(COMPOUND_STATEMENT, "compound-statement") \
  s(END, "end") \


typedef enum {
#define s(st, str) S_##st,
FACIO_STATE_LIST
#undef s
} facio_state;

static char *facio_state_strings[] = {
#define s(st, str) str,
FACIO_STATE_LIST
#undef s
};

static inline const char* facio_state_string(facio_state state){
  assert(state <= S_END);
  return facio_state_strings[state];
}

typedef struct {
  uint32_t line_num_, column_num_;
  const char* filename;
  FILE* file;
  facio_token_t tok, next;
  struct {
    uint8_t top;
    uint8_t* array;
  } indent_stack;
} facio_lexer;

bool facio_lexer_init(facio_lexer *self, const char* filename);

void facio_stack_push(facio_lexer *lexer, uint8_t value);
uint8_t facio_stack_pop(facio_lexer *lexer);
uint8_t facio_stack_peek(facio_lexer *lexer);

facio_token_t facio_scan(facio_lexer *lexer);
facio_token_t facio_peek(facio_lexer *lexer); // TODO

facio_token_t scan_string(facio_lexer *lexer, char quote);
facio_token_t scan_ident(facio_lexer *lexer, char c);
facio_token_t scan_number(facio_lexer *lexer, char c);
facio_token_t scan_indent(facio_lexer *lexer, char c);


#endif // FACIO_LEXER_H_
