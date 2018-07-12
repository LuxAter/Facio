#include "lexer.h"

#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "token.h"
#include "util.h"

facio_token_t set_ret(facio_lexer* lexer, facio_token_t tok)
{
  lexer->tok = tok;
  return tok;
}

bool facio_lexer_init(facio_lexer* self, const char* filename)
{
  self->filename = strdup(filename);
  self->line_num_ = 0;
  self->column_num_ = 0;
  self->indent_stack.top = 0;
  self->indent_stack.array = (uint8_t*)malloc(128 * sizeof(uint8_t));
  self->file = fopen(filename, "r");
  if (!self->file) {
    return false;
    self->file = NULL;
  } else {
    return true;
  }
}

void facio_stack_push(facio_lexer* lexer, uint8_t value)
{
  if (lexer->indent_stack.top != 128) {
    lexer->indent_stack.array[++(lexer->indent_stack.top) - 1] = value;
  }
}
uint8_t facio_stack_pop(facio_lexer* lexer)
{
  if (lexer->indent_stack.top != 0) {
    return lexer->indent_stack.array[(lexer->indent_stack.top)-- - 1];
  }
  return 0;
}
uint8_t facio_stack_peek(facio_lexer* lexer)
{
  if (lexer->indent_stack.top != 0) {
    return lexer->indent_stack.array[lexer->indent_stack.top - 1];
  }
  return 0;
}

facio_token_t facio_scan(facio_lexer* lexer)
{
  int c;
scan:
  if (lexer->tok.type == EOL || lexer->tok.type == DEDENT) {
    facio_token_t tok = scan_indent(lexer, c);
    if (tok.type != NILL) {
      return set_ret(lexer, tok);
    }
  }
  switch (c = fgetc(lexer->file)) {
  case ' ':
  case '\t':
    goto scan;
  case '\n':
  case '\r':
    return set_ret(lexer, facio_get_token(EOL, "EOL"));
  case '(':
    return set_ret(lexer, facio_get_token(LPAREN, "("));
  case ')':
    return set_ret(lexer, facio_get_token(RPAREN, ")"));
  case '{':
    return set_ret(lexer, facio_get_token(LBRACE, "{"));
  case '}':
    return set_ret(lexer, facio_get_token(RBRACE, "}"));
  case ':':
    return set_ret(lexer, facio_get_token(COLON, ":"));
  case '=':
    if ('=' == fpeek(lexer->file)) {
      fgetc(lexer->file);
      return set_ret(lexer, facio_get_token(OP_EQ, "=="));
    } else {
      return set_ret(lexer, facio_get_token(OP_ASSIGN, "="));
    }
  case EOF:
    return set_ret(lexer, facio_get_token(EOS, "EOF"));
  case '"':
  case '\'':
    return set_ret(lexer, scan_string(lexer, c));
  default:
    if (isalpha(c) || c == '_')
      return set_ret(lexer, scan_ident(lexer, c));
    if (isdigit(c) || c == '.')
      return set_ret(lexer, scan_number(lexer, c));
    return set_ret(lexer, facio_get_token(ILLEGAL, "illegal"));
  }
}

facio_token_t scan_string(facio_lexer* lexer, char quote)
{
  char c;
  int len = 0;
  char buf[255];
  while (quote != (c = fgetc(lexer->file))) {
    switch (c) {
    case '\n':
      ++lexer->line_num_;
      break;
    case '\\':
      switch (c = fgetc(lexer->file)) {
      case 'a':
        c = '\a';
        break;
      case 'b':
        c = '\b';
        break;
      case 'f':
        c = '\f';
        break;
      case 'n':
        c = '\n';
        break;
      case 'r':
        c = '\r';
        break;
      case 't':
        c = '\t';
        break;
      case 'v':
        c = '\v';
        break;
      }
      break;
    }
    buf[len++] = c;
  }
  buf[len++] = 0;
  return facio_get_token(STRING, buf);
}
facio_token_t scan_ident(facio_lexer* lexer, char c)
{
  int len = 0;
  char buf[255];
  do {
    buf[len++] = c;
  } while (isalpha(c = fgetc(lexer->file)) || isdigit(c) || c == '_');
  ungetc(c, lexer->file);
  buf[len++] = 0;
  switch (len - 1) {
  case 2:
    if (strncmp(buf, "if", len) == 0) {
      return facio_get_token(IF, "if");
    }
  case 3:
    if (strncmp(buf, "for", len) == 0) {
      return facio_get_token(FOR, "for");
    }
    if (strncmp(buf, "def", len) == 0) {
      return facio_get_token(DEF, "def");
    }
  case 4:
    if (strncmp(buf, "else", len) == 0) {
      return facio_get_token(ELSE, "else");
    }
    if (strncmp(buf, "elif", len) == 0) {
      return facio_get_token(ELIF, "elif");
    }
  case 5:
    if (strncmp(buf, "while", len) == 0) {
      return facio_get_token(WHILE, "while");
    }
  case 6:
    if (strncmp(buf, "return", len) == 0) {
      return facio_get_token(RETURN, "return");
    }
  default:
    return facio_get_token(ID, buf);
  }
}
facio_token_t scan_number(facio_lexer* lexer, char c)
{
  int n = 0, type = 0, expo = 0, e, expo_type = 1;
  do {
    if (c == '_') {
      continue;
    }
    if (c == '.') {
      goto scan_float;
    } else if (c == 'e' || c == 'E') {
      goto scan_expo;
    }
    n = n * 10 + c - '0';
  } while (isdigit(c = fgetc(lexer->file)) || c == '_' || c == '.' || c == 'e' || c == 'E');
  return facio_get_token_int(INT, n);
scan_float:
  e = 1;
  type = 1;
  while (isdigit(c = fgetc(lexer->file)) || c == '_' || c == 'e' || c == 'E') {
    if (c == '_') {
      continue;
    }
    if (c == 'e' || c == 'E') {
      goto scan_expo;
    }
    n = n * 10 + c - '0';
    e *= 10;
  }
  return facio_get_token_float(FLOAT, (double)n / e);
scan_expo:
  while (isdigit(c = fgetc(lexer->file)) || c == '+' || c == '-') {
    if (c == '-') {
      expo_type = 0;
      continue;
    }
    expo = expo * 10 + c - '0';
  }
  if (expo_type == 0) {
    expo *= -1;
  }
  if (type == 0) {
    return facio_get_token_int(INT, n * pow(10, expo));
  }
  return facio_get_token_float(FLOAT, ((double)n / e) * pow(10, expo));
}

facio_token_t scan_indent(facio_lexer* lexer, char c)
{
  uint8_t count = 0;
  do {
    if (c == ' ') {
      count++;
    } else if (c == '\t') {
      count += 8;
    }
  } while ((c = fgetc(lexer->file)) == ' ' || c == '\t');
  ungetc(c, lexer->file);
  char buf[128];
  uint8_t top = facio_stack_peek(lexer);
  if (count == 0 && lexer->indent_stack.top != 0) {
    facio_stack_pop(lexer);
    return facio_get_token(DEDENT, "0");
  }
  if (count > top) {
    facio_stack_push(lexer, count);
    sprintf(buf, "%d", count);
    return facio_get_token(INDENT, buf);
  } else if (count < top) {
    facio_stack_pop(lexer);
    if (count < facio_stack_peek(lexer)) {
      for (uint8_t i = 0; i < count; ++i) {
        ungetc(' ', lexer->file);
      }
      sprintf(buf, "%d", facio_stack_peek(lexer));
      return facio_get_token(DEDENT, buf);
    }
    sprintf(buf, "%d", count);
    return facio_get_token(DEDENT, buf);
  }
  return facio_get_token(NILL, "null");
}
