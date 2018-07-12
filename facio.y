%{
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "output.h"
#include "file.h"

extern int yylex();
extern int yyparse();
extern FILE* yyin;
extern uint32_t column_;
extern uint32_t line_num_;
extern char* yytext;

void yyerror(const char* s);
%}

%token IDENTIFIER

%token INT FLOAT STRING_LITERAL

%token INDENT DEDENT
%token IF ELIF ELSE
%token WHILE
%token OR_OP AND_OP EQ_OP NE_OP LE_OP GE_OP MOD_OP

%start translation_unit 

%%

translation_unit: statement_list
                ;

statement_list: statement {printf("SINGLE\n");}
              | statement_list statement {printf("MULTIPLE\n");}
              ;

statement: selection_statement 
         | iteration_statement
         | expression {printf("Expression\n");}
         ;

selection_statement: IF '(' expression ')' block {printf("selc[0]\n");}
                   | selection_statement ELIF '(' expression ')' block
                   | selection_statement ELSE block
                   ;

iteration_statement: WHILE '(' expression ')' block

expression: assignment_expression
          ;

assignment_expression: conditional_expression {printf("assign[0]\n");}
                     | postfix_expression '=' assignment_expression {printf("assignment[1]\n");}
                     ;

conditional_expression: logical_or_expression {printf("cond[0]\n");}
                      | logical_or_expression '?' expression ':' conditional_expression 
                      ;

logical_or_expression: logical_and_expression {printf("or[0]\n");}
                     | logical_or_expression OR_OP logical_and_expression
                     ;

logical_and_expression: equality_expression {printf("and[0]\n");}
                      | logical_and_expression AND_OP equality_expression
                      ;

equality_expression: relational_expression {printf("eq[0]\n");}
                   | equality_expression EQ_OP relational_expression {printf("eq[1]\n");}
                   | equality_expression NE_OP relational_expression
                   ;

relational_expression: additive_expression {printf("rel[0]\n");}
                     | relational_expression '<' additive_expression
                     | relational_expression '>' additive_expression
                     | relational_expression LE_OP additive_expression
                     | relational_expression GE_OP additive_expression {printf("rel[4]\n");}
                     ;

additive_expression: multiplicative_expression {printf("add[0]\n");}
                   | additive_expression '+' multiplicative_expression {printf("add[1]\n");}
                   | additive_expression '-' multiplicative_expression {printf("add[2]\n");}
                   ;

multiplicative_expression: postfix_expression {printf("mult[0]\n");}
                         | multiplicative_expression '*' postfix_expression  {printf("mult[1]\n");}
                         | multiplicative_expression '/' postfix_expression  {printf("mult[2]\n");}
                         | multiplicative_expression MOD_OP postfix_expression  {printf("mult[3]\n");}
                         ;

postfix_expression: primary_expression {printf("postfix[0]\n");}
                  | postfix_expression '(' argument_expression_list ')' {printf("postfix[1]\n");}
                  ;

primary_expression: IDENTIFIER {printf("ID\n");}
                  | constant {printf("constant\n");}
                  ;

constant: INT {$$ = "INT"; printf("INT\n");}
        | FLOAT {$$ = "FLOAT"; printf("FLOAT\n");}
        | STRING_LITERAL {$$ = "STRING"; printf("STRING_LITERAL\n");}
        ;

argument_expression_list: assignment_expression
                        | argument_expression_list ',' assignment_expression
                        ;



block: '{' '}' {printf("BLOCK[0]\n");}
     | '{' statement_list '}'  {printf("BLOCK[1]\n");}

%%

int main(int argc, const char* argv[]){
  FILE* file = open_file();
  if(!file){
    return 1;
  }
  yyin = file;
  do {
    yyparse();
  } while(!feof(yyin));
  fclose(file);
}


void yyerror(const char* s){
  fatal("<Parse Error[%u,%u]> %s", line_num_, column_, s);
  exit(-1);
}
