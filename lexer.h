#ifndef FACIO_LEXER_H_
#define FACIO_LEXER_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "token.h"

typedef struct {
  uint32_t line_num_, column_num_;
  const char* filename;
  FILE* file;
  facio_token_t tok;
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

facio_token_t scan_string(facio_lexer *lexer, char quote);
facio_token_t scan_ident(facio_lexer *lexer, char c);
facio_token_t scan_number(facio_lexer *lexer, char c);
facio_token_t scan_indent(facio_lexer *lexer, char c);


#endif // FACIO_LEXER_H_
