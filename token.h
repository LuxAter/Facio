#ifndef FACIO_TOKEN_H_
#define FACIO_TOKEN_H_

#include <assert.h>
#include <string.h>
#include <stdio.h>

#define FACIO_TOKEN_LIST \
  t(ILLEGAL, "illegal") \
  t(ID, "id") \
  t(INT, "int") \
  t(FLOAT, "float") \
  t(STRING, "string") \
  t(IF, "if") \
  t(ELSE, "else") \
  t(ELIF, "elif") \
  t(WHILE, "while") \
  t(FOR, "for") \
  t(RETURN, "return") \
  t(LBRACE, "{") \
  t(RBRACE, "}") \
  t(LPAREN, "(") \
  t(RPAREN, ")") \
  t(LBRACK, "[") \
  t(RBRACK, "]") \
  t(COLON, ":") \
  t(QMARK, "?") \
  t(OP_EQ, "==") \
  t(OP_ASSIGN, "=") \
  t(SEMICOLON, ";") \
  t(COMMA, ",") \
  t(INDENT, "->") \
  t(DEDENT, "<-") \
  t(NILL, "null") \
  t(EOL, "end-of-line") \
  t(EOS, "end-of-source") \

typedef enum {
#define t(tok, str) tok,
FACIO_TOKEN_LIST
#undef t
} facio_token;

static char *facio_token_strings[] = {
#define t(tok, str) str,
FACIO_TOKEN_LIST
#undef t
};

typedef struct {
  int len;
  const char* src;
  struct {
    long int int_;
    double float_;
    const char* string_;
  } value;
  facio_token type;
} facio_token_t;

static inline const char* facio_token_type_string(facio_token type){
  assert(type <= EOS);
  return facio_token_strings[type];
}

static inline facio_token_t facio_get_token(facio_token type, const char* src){
  return (facio_token_t){strlen(src), strdup(src), {0, 0.0, ""}, type};
}
static inline facio_token_t facio_get_token_int(facio_token type, long int src){
  char buf[255];
  sprintf(buf, "%ld", src);
  return (facio_token_t){strlen(buf), strdup(buf), {src, 0.0, ""}, type};
}
static inline facio_token_t facio_get_token_float(facio_token type, double src){
  char buf[255];
  sprintf(buf, "%lf", src);
  return (facio_token_t){strlen(buf), strdup(buf), {0, src, ""}, type};
}
static inline facio_token_t facio_get_token_string(facio_token type, const char* src){
  return (facio_token_t){strlen(src), strdup(src), {0, 0.0, strdup(src)}, type};
}


#endif  // FACIO_TOKEN_H_
