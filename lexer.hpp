#ifndef FACIO_LEXER_HPP_
#define FACIO_LEXER_HPP_

#include <cstdio>
#include <cstdint>
#include <stack>

#include "token.hpp"

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
  return facio_state_strings[state];
}

struct Lexer {
  Lexer(std::string file_name);
  uint32_t line_num_, column_num_;
  std::string filename;
  FILE* file;
  Token tok, next;
  std::stack<uint8_t> indent;
  Token Scan();
  Token ScanString(char quote);
  Token ScanIdentifier(char c);
  Token ScanNumber(char c);
  Token ScanIndent(char c);
};


#endif // FACIO_LEXER_HPP_
