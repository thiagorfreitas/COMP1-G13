%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int yylex(void);
void yyerror(const char *s);
%}

%union {
    char* str;
}

%start programa

// Tokens com valor associado (strings, ids, números)
%token <str> ID NUMBER STRING CHAR

// Tipos e palavras-chave
%token KW_INT KW_FLOAT KW_CHAR KW_DOUBLE KW_RETURN
%token KW_IF KW_ELSE KW_WHILE KW_FOR KW_DO
%token KW_VOID KW_LONG KW_SHORT KW_SIGNED KW_UNSIGNED
%token KW_BREAK KW_CASE KW_CONST KW_CONTINUE KW_DEFAULT
%token KW_ENUM KW_STATIC KW_SIZEOF KW_SWITCH

// Operadores e símbolos
%token OP_ASSIGN OP_EQ OP_NEQ OP_LEQ OP_GEQ OP_AND OP_OR OP_INC OP_DEC
%token OP_PLUS OP_MINUS OP_MUL OP_DIV
%token OP_LT OP_GT OP_BIT_AND OP_BIT_OR OP_BIT_XOR OP_BIT_NOT OP_NOT

%token SEMICOLON COMMA LPAREN RPAREN LBRACE RBRACE LBRACKET RBRACKET

%type <str> valor

%type <str> tipo

%%

programa:
    lista_comandos
    ;

lista_comandos:
    lista_comandos comando
    | comando
    ;

comando:
    declaracao_var
    | atribuicao
    | print
    ;

declaracao_var:
    tipo ID SEMICOLON {
        printf("[DECL] Tipo: %s, Nome: %s\n", $1, $2);
    }
    ;

atribuicao:
    ID OP_ASSIGN valor SEMICOLON {
        printf("[ATRIB] %s = %s\n", $1, $3);
    }
    ;

print:
    ID LPAREN STRING COMMA ID RPAREN SEMICOLON {
        printf("[PRINT] Conteúdo de %s: %s\n", $5, $3);
    }
    ;

valor:
    NUMBER   { $$ = $1; }
    | CHAR   { $$ = $1; }
    | STRING { $$ = $1; }
    ;

tipo:
    KW_INT     { $$ = "int"; }
    | KW_FLOAT { $$ = "float"; }
    | KW_CHAR  { $$ = "char"; }
    | KW_DOUBLE { $$ = "double"; }
    | KW_VOID  { $$ = "void"; }
    ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Erro de sintaxe: %s\n", s);
}
