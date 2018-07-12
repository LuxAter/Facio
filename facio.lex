%option noyywrap

D [0-9]
L [a-zA-Z_]

%{
#include <stdint.h>
#include "facio.tab.h"
uint32_t column_ = 0;
uint32_t line_num_ = 1;
uint16_t current_indent_ = 0;

void count();
%}

%%

"if" {count(); return (IF);}
"while" {count(); return (WHILE);}
("else if"|"elif") {count(); return(ELIF);}
"else" {count(); return (ELSE);}
("or"|"||") {count(); return (OR_OP);}
("and"|"&&") {count(); return (AND_OP);}
("is"|"==") {count(); return (EQ_OP);}
("is not"|"!=") {count(); return (NE_OP);}
"<=" {count(); return (LE_OP);}
">=" {count(); return (GE_OP);}
("mod"|"%") {count(); return(MOD_OP);}

{L}({L}|{D})* { count(); return IDENTIFIER; }
{D}+.{D}+ {count(); return FLOAT;}
{D}+ {count(); return INT;}
L?(\"|\')(\\.|[^\\"])*(\"|\') {count(); return STRING_LITERAL; }

"[" {count(); return ('[');}
"]" {count(); return (']');}
"(" {count(); return ('(');}
")" {count(); return (')');}
"{" {count(); return ('{');}
"}" {count(); return ('}');}
"<" {count(); return ('<');}
">" {count(); return ('>');}
"=" {count(); return ('=');}
"," {count(); return (',');}
":" {count(); return (':');}
"?" {count(); return ('?');}
"+" {count(); return ('+');}
"-" {count(); return ('-');}
"*" {count(); return ('*');}
"/" {count(); return ('/');}

[ \t\v\n\f] {count();} 

. ;

%%

void count() {
  for(int i = 0; yytext[i] != '\0'; ++i){
    if(yytext[i] == '\n'){
      column_ = 0;
      line_num_++;
    }else if(yytext[i] == '\t'){
      column_ += 8 - (column_ % 8);
    }else{
      column_++;
    }
  }
}
