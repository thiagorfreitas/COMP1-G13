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

/* Tokens com valor  (strings, ids, números)*/
%token <str> ID NUMBER STRING CHAR

/* Palavras-chave */
%token KW_INT KW_FLOAT KW_CHAR KW_DOUBLE KW_RETURN
%token KW_IF KW_ELSE KW_WHILE KW_FOR KW_DO
%token KW_VOID KW_LONG KW_SHORT KW_SIGNED KW_UNSIGNED
%token KW_BREAK KW_CASE KW_CONST KW_CONTINUE KW_DEFAULT
%token KW_ENUM KW_STATIC KW_SIZEOF KW_SWITCH

/* Operadores e símbolos */
%token OP_ASSIGN OP_EQ OP_NEQ OP_LEQ OP_GEQ OP_AND OP_OR OP_INC OP_DEC
%token OP_PLUS OP_MINUS OP_MUL OP_DIV
%token OP_LT OP_GT OP_BIT_AND OP_BIT_OR OP_BIT_XOR OP_BIT_NOT OP_NOT

%token SEMICOLON COMMA LPAREN RPAREN LBRACE RBRACE LBRACKET RBRACKET

%type <str> valor tipo expr

/* Precedência e associatividade para expressões */
%left OP_OR
%left OP_AND
%left OP_EQ OP_NEQ
%left OP_LT OP_GT OP_LEQ OP_GEQ
%left OP_PLUS OP_MINUS
%left OP_MUL OP_DIV

%%

programa:
    lista_comandos
    ;

lista_comandos:
    /* vazio */
  | lista_comandos comando
    ;

comando:
    declaracao_var
  | atribuicao
  | print
  | KW_IF LPAREN expr RPAREN comando    { printf("[IF]\n"); }
  | KW_WHILE LPAREN expr RPAREN comando { printf("[WHILE]\n"); }
    ;

declaracao_var:
    tipo ID SEMICOLON {
        printf("[DECL] Tipo: %s, Nome: %s\n", $1, $2);
    }
    ;

atribuicao:
    ID OP_ASSIGN expr SEMICOLON {
        printf("[ATRIB] %s = %s\n", $1, $3);
    }
    ;

print:
    ID LPAREN STRING COMMA ID RPAREN SEMICOLON {
        printf("[PRINT] Conteudo de %s: %s\n", $5, $3);
    }
    ;

expr:
    valor                   { $$ = $1; }
  | ID                      { $$ = strdup($1); }
  | expr OP_PLUS expr       { $$ = strdup("(expr+expr)"); }
  | expr OP_MINUS expr      { $$ = strdup("(expr-expr)"); }
  | expr OP_MUL expr        { $$ = strdup("(expr*expr)"); }
  | expr OP_DIV expr        { $$ = strdup("(expr/expr)"); }
  | LPAREN expr RPAREN      { $$ = $2; }
    ;

valor:
    NUMBER   { $$ = $1; }
  | CHAR     { $$ = $1; }
  | STRING   { $$ = $1; }
    ;

tipo:
    KW_INT     { $$ = "int"; }
  | KW_FLOAT   { $$ = "float"; }
  | KW_CHAR    { $$ = "char"; }
  | KW_DOUBLE  { $$ = "double"; }
  | KW_VOID    { $$ = "void"; }
    ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Erro de sintaxe: %s\n", s);
}
