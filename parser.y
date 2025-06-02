%{
#include "ast2.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tabela.h"

NoAST* raizAST = NULL;
int yylex(void);
void yyerror(const char *s);
%}

%union {
    char* str;
    NoAST* ast;
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
%token OP_MOD
%token OP_LT OP_GT OP_BIT_AND OP_BIT_OR OP_BIT_XOR OP_BIT_NOT OP_NOT

%token SEMICOLON COMMA LPAREN RPAREN LBRACE RBRACE LBRACKET RBRACKET

%type <ast> programa lista_comandos comando declaracao_var atribuicao print bloco if_else while_loop for_loop do_while_loop expr valor
%type <str> tipo

/* Precedência e associatividade para expressões */
%left OP_OR
%left OP_AND
%left OP_EQ OP_NEQ
%left OP_LT OP_GT OP_LEQ OP_GEQ
%left OP_PLUS OP_MINUS
%left OP_MUL OP_DIV OP_MOD


%%

programa:
    lista_comandos { raizAST = $1; }
    ;

lista_comandos:
      comando { $$ = criarNo(AST_BLOCO, NULL, NULL, 1, $1); }
    | lista_comandos comando {
        // Adiciona novo comando ao bloco
        NoAST** filhos = malloc(($1->n_filhos + 1) * sizeof(NoAST*));
        for (int i = 0; i < $1->n_filhos; i++) filhos[i] = $1->filhos[i];
        filhos[$1->n_filhos] = $2;
        $1->filhos = filhos;
        $1->n_filhos++;
        $$ = $1;
      }
    ;

comando:
    declaracao_var
  | atribuicao
  | print
  | bloco
  | if_else
  | while_loop
  | for_loop
  | do_while_loop
  ;

declaracao_var:
    tipo ID SEMICOLON {
        if (buscar_simbolo($2)) {
            printf("Erro: variável '%s' já declarada!\n", $2);
            $$ = criarNo(AST_DECL, $2, "erro", 0);
        } else {
            adicionar_simbolo($2, $1);
            printf("[DECL] Tipo: %s, Nome: %s\n", $1, $2);
            $$ = criarNo(AST_DECL, $2, $1, 0);
        }
    }
    ;

atribuicao:
    ID OP_ASSIGN expr SEMICOLON {
        Simbolo* sim = buscar_simbolo($1);
        char* tipoDado = (!sim) ? "erro" : sim->tipo;
        if (!sim) {
            printf("Erro: variável '%s' não declarada!\n", $1);
        } else if (strcmp(tipoDado, $3->tipoDado) != 0) {
            printf("Erro: atribuição incompatível! Variável '%s' é do tipo '%s', valor é do tipo '%s'.\n", $1, tipoDado, $3->tipoDado);
        } else {
            printf("[ATRIB] %s = ...\n", $1);
        }
        $$ = criarNo(AST_ATRIB, $1, tipoDado, 1, $3);
    }
    ;



print:
    ID LPAREN STRING COMMA ID RPAREN SEMICOLON {
        printf("[PRINT] Conteudo de %s: %s\n", $5, $3);
        $$ = criarNo(AST_PRINT, $5, NULL, 0);
    }
    ;


bloco:
    LBRACE lista_comandos RBRACE { $$ = $2; }
    ;


if_else:
    KW_IF LPAREN expr RPAREN comando {
        printf("[IF]\n");
        $$ = criarNo(AST_IF, NULL, NULL, 2, $3, $5);
    }
  | KW_IF LPAREN expr RPAREN comando KW_ELSE comando {
        printf("[IF/ELSE]\n");
        $$ = criarNo(AST_IF_ELSE, NULL, NULL, 3, $3, $5, $7);
    }
    ;


while_loop:
    KW_WHILE LPAREN expr RPAREN comando {
        printf("[WHILE]\n");
        $$ = criarNo(AST_WHILE, NULL, NULL, 2, $3, $5);
    }
    ;


for_loop:
    KW_FOR LPAREN atribuicao expr SEMICOLON atribuicao RPAREN comando {
        printf("[FOR]\n");
        $$ = criarNo(AST_FOR, NULL, NULL, 4, $3, $4, $6, $8);
    }
    ;


do_while_loop:
    KW_DO comando KW_WHILE LPAREN expr RPAREN SEMICOLON {
        printf("[DO_WHILE]\n");
        $$ = criarNo(AST_DO_WHILE, NULL, NULL, 2, $2, $5);
    }
    ;

expr:
      valor { $$ = $1; }
    | ID {
          Simbolo* sim = buscar_simbolo($1);
          char* tipoDado = (!sim) ? "erro" : sim->tipo;
          if (!sim)
            printf("Erro: variável '%s' não declarada!\n", $1);
          $$ = criarNo(AST_ID, $1, tipoDado, 0);
      }
    | expr OP_PLUS expr {
          char* tipo_result = NULL;
          if (strcmp($1->tipoDado, "int") == 0 && strcmp($3->tipoDado, "int") == 0)
              tipo_result = "int";
          else if (
              (strcmp($1->tipoDado, "float") == 0 && strcmp($3->tipoDado, "int") == 0) ||
              (strcmp($1->tipoDado, "int") == 0 && strcmp($3->tipoDado, "float") == 0) ||
              (strcmp($1->tipoDado, "float") == 0 && strcmp($3->tipoDado, "float") == 0)
          )
              tipo_result = "float";
          else
              tipo_result = "erro";
          $$ = criarNo(AST_EXPR, "+", tipo_result, 2, $1, $3);
      }
    | expr OP_MINUS expr {
          char* tipo_result = NULL;
          if (strcmp($1->tipoDado, "int") == 0 && strcmp($3->tipoDado, "int") == 0)
              tipo_result = "int";
          else if (
              (strcmp($1->tipoDado, "float") == 0 && strcmp($3->tipoDado, "int") == 0) ||
              (strcmp($1->tipoDado, "int") == 0 && strcmp($3->tipoDado, "float") == 0) ||
              (strcmp($1->tipoDado, "float") == 0 && strcmp($3->tipoDado, "float") == 0)
          )
              tipo_result = "float";
          else
              tipo_result = "erro";
          $$ = criarNo(AST_EXPR, "-", tipo_result, 2, $1, $3);
      }
    | expr OP_MUL expr {
          char* tipo_result = NULL;
          if (strcmp($1->tipoDado, "int") == 0 && strcmp($3->tipoDado, "int") == 0)
              tipo_result = "int";
          else if (
              (strcmp($1->tipoDado, "float") == 0 && strcmp($3->tipoDado, "int") == 0) ||
              (strcmp($1->tipoDado, "int") == 0 && strcmp($3->tipoDado, "float") == 0) ||
              (strcmp($1->tipoDado, "float") == 0 && strcmp($3->tipoDado, "float") == 0)
          )
              tipo_result = "float";
          else
              tipo_result = "erro";
          $$ = criarNo(AST_EXPR, "*", tipo_result, 2, $1, $3);
      }
    | expr OP_DIV expr {
          char* tipo_result = NULL;
          if (strcmp($1->tipoDado, "int") == 0 && strcmp($3->tipoDado, "int") == 0)
              tipo_result = "int";
          else if (
              (strcmp($1->tipoDado, "float") == 0 && strcmp($3->tipoDado, "int") == 0) ||
              (strcmp($1->tipoDado, "int") == 0 && strcmp($3->tipoDado, "float") == 0) ||
              (strcmp($1->tipoDado, "float") == 0 && strcmp($3->tipoDado, "float") == 0)
          )
              tipo_result = "float";
          else
              tipo_result = "erro";
          $$ = criarNo(AST_EXPR, "/", tipo_result, 2, $1, $3);
      }
    | expr OP_MOD expr {
          char* tipo_result = NULL;
          if (strcmp($1->tipoDado, "int") == 0 && strcmp($3->tipoDado, "int") == 0)
              tipo_result = "int";
          else
              tipo_result = "erro";
          $$ = criarNo(AST_EXPR, "%", tipo_result, 2, $1, $3);
      }
    | LPAREN expr RPAREN { $$ = $2; }
    ;

valor:
      NUMBER   { $$ = criarNo(AST_NUM, $1, "int", 0); }
    | CHAR     { $$ = criarNo(AST_CHAR, $1, "char", 0); }
    | STRING   { $$ = criarNo(AST_STRING, $1, "string", 0); }
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

int main(void) {
    yyparse();
    imprimirTabela();
    printf("\n\n");
    imprimirAST(raizAST, 0);
    return 0;
}
