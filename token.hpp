#ifndef FACIO_TOKEN_H_
#define FACIO_TOKEN_H_

#include <string>

#define FACIO_TOKEN_LIST \
  t(ILLEGAL, "illegal") \
  t(IDENTIFIER, "identifier") \
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
  t(OP_ADD, "+") \
  t(OP_SUB, "-") \
  t(OP_MUL, "*") \
  t(OP_DIV, "/") \
  t(OP_MOD, "%") \
  t(OP_EQ, "==") \
  t(OP_L, "<") \
  t(OP_G, ">") \
  t(OP_LE, "<=") \
  t(OP_GE, ">=") \
  t(OP_ASSIGN, "=") \
  t(SEMICOLON, ";") \
  t(COMMA, ",") \
  t(DEF, "def") \
  t(INDENT, "->") \
  t(DEDENT, "<-") \
  t(NILL, "null") \
  t(EOL, "end-of-line") \
  t(EOS, "end-of-source") \

typedef enum {
#define t(tok, str) T_##tok,
FACIO_TOKEN_LIST
#undef t
} facio_token;

static std::string token_strings[] = {
#define t(tok, str) str,
FACIO_TOKEN_LIST
#undef t
};

struct Token {
  Token(facio_token type_, std::string src_): src(src_), type(type_){ }
  Token(long int src): src(std::to_string(src)), type(T_INT), int_(src){}
  Token(double src): src(std::to_string(src)), type(T_FLOAT), float_(src){}
  Token(std::string src): src(src), type(T_STRING), string_(src){}
  std::string src;
  long int int_;
  double float_;
  std::string string_;
  inline std::string String(){
    return token_strings[type];
  }
  facio_token type;
};

#endif  // FACIO_TOKEN_HPP_
