%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tabela.h"

int yylex(void);
void yyerror(const char *s);
%}

%union {
    char* str;
    struct {
        char* valor;
        char* tipo;
    } exprinfo;
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

%type <exprinfo> expr valor
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
        } else {
            adicionar_simbolo($2, $1);
            printf("[DECL] Tipo: %s, Nome: %s\n", $1, $2);
        }
    }
    ;

atribuicao:
    ID OP_ASSIGN expr SEMICOLON {
        Simbolo* sim = buscar_simbolo($1);
        if (!sim) {
            printf("Erro: variável '%s' não declarada!\n", $1);
        } else if (strcmp(sim->tipo, $3.tipo) != 0) {
            printf("Erro: atribuição incompatível! Variável '%s' é do tipo '%s', valor é do tipo '%s'.\n", $1, sim->tipo, $3.tipo);
        } else {
            printf("[ATRIB] %s = %s\n", $1, $3.valor);
        }
    }
    ;


print:
    ID LPAREN STRING COMMA ID RPAREN SEMICOLON {
        printf("[PRINT] Conteudo de %s: %s\n", $5, $3);
    }
    ;


bloco:
    LBRACE lista_comandos RBRACE
    ;


if_else:
    KW_IF LPAREN expr RPAREN comando {
        printf("[IF]\n");
    }
  | KW_IF LPAREN expr RPAREN comando KW_ELSE comando {
        printf("[IF/ELSE]\n");
    }
    ;


while_loop:
    KW_WHILE LPAREN expr RPAREN comando {
        printf("[WHILE]\n");
    }
    ;


for_loop:
    KW_FOR LPAREN atribuicao expr SEMICOLON atribuicao RPAREN comando {
        printf("[FOR]\n");
    }
    ;


do_while_loop:
    KW_DO comando KW_WHILE LPAREN expr RPAREN SEMICOLON {
        printf("[DO_WHILE]\n");
    }
    ;

expr:
      valor {
          $$.valor = $1.valor;
          $$.tipo  = $1.tipo;
      }
    | ID {
          Simbolo* sim = buscar_simbolo($1);
          if (!sim) {
              printf("Erro: variável '%s' não declarada!\n", $1);
              $$.tipo = "erro";
          } else {
              $$.tipo = sim->tipo;
          }
          $$.valor = strdup($1);
      }
    | expr OP_PLUS expr {
          // Exemplo só para int e float
          if (strcmp($1.tipo, "int") == 0 && strcmp($3.tipo, "int") == 0)
              $$.tipo = "int";
          else if ((strcmp($1.tipo, "float") == 0 && strcmp($3.tipo, "int") == 0) ||
                   (strcmp($1.tipo, "int") == 0 && strcmp($3.tipo, "float") == 0) ||
                   (strcmp($1.tipo, "float") == 0 && strcmp($3.tipo, "float") == 0))
              $$.tipo = "float";
          else
              $$.tipo = "erro";
          $$.valor = strdup("(expr+expr)");
      }
    | expr OP_MINUS expr {
          if (strcmp($1.tipo, "int") == 0 && strcmp($3.tipo, "int") == 0)
              $$.tipo = "int";
          else if ((strcmp($1.tipo, "float") == 0 && strcmp($3.tipo, "int") == 0) ||
                   (strcmp($1.tipo, "int") == 0 && strcmp($3.tipo, "float") == 0) ||
                   (strcmp($1.tipo, "float") == 0 && strcmp($3.tipo, "float") == 0))
              $$.tipo = "float";
          else
              $$.tipo = "erro";
          $$.valor = strdup("(expr-expr)");
      }
    | expr OP_MUL expr {
          if (strcmp($1.tipo, "int") == 0 && strcmp($3.tipo, "int") == 0)
              $$.tipo = "int";
          else if ((strcmp($1.tipo, "float") == 0 && strcmp($3.tipo, "int") == 0) ||
                   (strcmp($1.tipo, "int") == 0 && strcmp($3.tipo, "float") == 0) ||
                   (strcmp($1.tipo, "float") == 0 && strcmp($3.tipo, "float") == 0))
              $$.tipo = "float";
          else
              $$.tipo = "erro";
          $$.valor = strdup("(expr*expr)");
      }
    | expr OP_DIV expr {
          if (strcmp($1.tipo, "int") == 0 && strcmp($3.tipo, "int") == 0)
              $$.tipo = "int";
          else if ((strcmp($1.tipo, "float") == 0 && strcmp($3.tipo, "int") == 0) ||
                   (strcmp($1.tipo, "int") == 0 && strcmp($3.tipo, "float") == 0) ||
                   (strcmp($1.tipo, "float") == 0 && strcmp($3.tipo, "float") == 0))
              $$.tipo = "float";
          else
              $$.tipo = "erro";
          $$.valor = strdup("(expr/expr)");
      }
    | expr OP_MOD expr {
          if (strcmp($1.tipo, "int") == 0 && strcmp($3.tipo, "int") == 0)
              $$.tipo = "int";
          else
              $$.tipo = "erro";
          $$.valor = strdup("(expr%expr)");
      }
    | LPAREN expr RPAREN {
          $$.valor = $2.valor;
          $$.tipo  = $2.tipo;
      }
    ;

valor:
      NUMBER   { $$.valor = $1; $$.tipo = "int"; }
    | CHAR     { $$.valor = $1; $$.tipo = "char"; }
    | STRING   { $$.valor = $1; $$.tipo = "string"; }
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
    return 0;
}
