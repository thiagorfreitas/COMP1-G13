%{
#include <stdio.h>
#include <stdlib.h>

int yylex(void);
void yyerror(const char *s);
%}

%union {
    char* str;
}

%token <str> ID NUMBER
%token INT FLOAT CHAR DOUBLE LONGLONG
%token IF ELSE WHILE RETURN
%token ASSIGN EQ OP
%token SEMI COMMA LPAREN RPAREN LBRACE RBRACE

%start programa

%%

programa:
    declaracoes
    ;

declaracoes:
    declaracoes declaracao
    | declaracao
    ;

declaracao:
    tipo ID SEMI {
        printf("Declaração de variável: tipo = %s, nome = %s\n", $1, $2);
    }
    ;

tipo:
    INT       { $$ = "int"; }
    | FLOAT   { $$ = "float"; }
    | CHAR    { $$ = "char"; }
    | DOUBLE  { $$ = "double"; }
    | LONGLONG { $$ = "long long"; }
    ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Erro de sintaxe: %s\n", s);
}
