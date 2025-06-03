%{
#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tabela.h"
#include "codegen.h" // Incluir header da geração de código

NoAST* raizAST = NULL;
int yylex(void);
void yyerror(const char *s);

// Função auxiliar para verificar compatibilidade de tipos aritméticos
// Retorna o tipo resultante ("int", "float") ou "erro"
const char* check_arithmetic_types(const char* type1, const char* type2, const char* op) {
    if (!type1 || !type2 || strcmp(type1, "erro") == 0 || strcmp(type2, "erro") == 0) {
        return "erro"; // Propaga erro
    }
    if (strcmp(type1, "int") == 0 && strcmp(type2, "int") == 0) {
        return "int";
    }
    if ((strcmp(type1, "float") == 0 || strcmp(type1, "int") == 0) && 
        (strcmp(type2, "float") == 0 || strcmp(type2, "int") == 0)) {
        // Permitir int/float misturado, resultado é float
        // Exceto para MOD, que exige ints
        if (strcmp(op, "%") == 0) {
             if (strcmp(type1, "int") == 0 && strcmp(type2, "int") == 0) {
                 return "int";
             } else {
                 fprintf(stderr, "Erro Semântico: Operador '%%' requer operandos inteiros, obteve '%s' e '%s'.\n", type1, type2);
                 return "erro";
             }
        }
        return "float"; 
    }
    fprintf(stderr, "Erro Semântico: Tipos incompatíveis para operador '%s': '%s' e '%s'.\n", op, type1, type2);
    return "erro";
}

// Função auxiliar para verificar compatibilidade de tipos para comparação/lógicos
// Retorna "int" (para booleano) se compatível, senão "erro"
const char* check_comparison_logical_types(const char* type1, const char* type2, const char* op) {
    if (!type1 || !type2 || strcmp(type1, "erro") == 0 || strcmp(type2, "erro") == 0) {
        return "erro"; // Propaga erro
    }
    // Permitir comparação entre int e float
    if ((strcmp(type1, "int") == 0 || strcmp(type1, "float") == 0) && 
        (strcmp(type2, "int") == 0 || strcmp(type2, "float") == 0)) {
        return "int"; // Resultado de comparação é booleano (int em C)
    }
    // Poderia adicionar comparação de char aqui se necessário
    fprintf(stderr, "Erro Semântico: Tipos incompatíveis para operador '%s': '%s' e '%s'.\n", op, type1, type2);
    return "erro";
}


%}

%union {
    char* str;
    NoAST* ast;
}


%start programa

/* Tokens com valor (strings, ids, números)*/
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
%right OP_ASSIGN
%left OP_OR
%left OP_AND
%left OP_EQ OP_NEQ
%left OP_LT OP_GT OP_LEQ OP_GEQ
%left OP_PLUS OP_MINUS
%left OP_MUL OP_DIV OP_MOD
%right OP_NOT OP_BIT_NOT OP_INC OP_DEC /* Unários */


%%

programa:
    lista_comandos { raizAST = $1; }
    ;

lista_comandos:
      comando { 
          if ($1) $$ = criarNo(AST_BLOCO, NULL, NULL, 1, $1); 
          else $$ = criarNo(AST_BLOCO, NULL, NULL, 0); // Bloco vazio se comando falhou
      }
    | lista_comandos comando {
        if ($1 && $2) { // Evita adicionar nós nulos se houve erro
            NoAST** filhos = realloc($1->filhos, ($1->n_filhos + 1) * sizeof(NoAST*));
            if (filhos) {
                $1->filhos = filhos;
                filhos[$1->n_filhos] = $2;
                $1->n_filhos++;
            } else {
                fprintf(stderr, "Erro: Falha ao realocar filhos da AST.\n");
                // Tratar erro de alocação se necessário (liberar $1?)
                $$ = NULL; // Sinaliza erro
                YYERROR;
            }
        } else if (!$1) {
             // Se a lista anterior já era inválida, propaga
             $$ = NULL;
        } else {
             // Se o comando atual é inválido, mantém a lista anterior válida
             $$ = $1;
        }
      }
    ;

comando:
    declaracao_var { $$ = $1; }
  | atribuicao { $$ = $1; }
  | print { $$ = $1; }
  | bloco { $$ = $1; }
  | if_else { $$ = $1; }
  | while_loop { $$ = $1; }
  | for_loop { $$ = $1; }
  | do_while_loop { $$ = $1; }
  | SEMICOLON { $$ = NULL; } /* Comando vazio, não gera nó */
  ;

declaracao_var:
    tipo ID SEMICOLON {
        if (buscar_simbolo($2)) {
            fprintf(stderr, "Erro Semântico: Variável '%s' já declarada neste escopo.\n", $2);
            $$ = NULL; // Retorna NULL para indicar erro
            YYERROR; // Sinaliza erro para Bison
        } else {
            adicionar_simbolo($2, $1);
            // printf("[DECL] Tipo: %s, Nome: %s\n", $1, $2); // Mensagem movida para codegen se necessário
            $$ = criarNo(AST_DECL, $2, $1, 0);
        }
        free($2); // Libera memória do ID copiado pelo lexer
    }
    ;

atribuicao:
    ID OP_ASSIGN expr SEMICOLON {
        Simbolo* sim = buscar_simbolo($1);
        if (!sim) {
            fprintf(stderr, "Erro Semântico: Variável '%s' não declarada.\n", $1);
            $$ = NULL;
            YYERROR;
        } else if (!$3) { // Verifica se a expressão teve erro
             // fprintf(stderr, "Erro Semântico: Erro na expressão da atribuição para '%s'.\n", $1);
             $$ = NULL;
             // YYERROR já deve ter sido chamado na expr
        } else if (strcmp($3->tipoDado, "erro") == 0) {
             // fprintf(stderr, "Erro Semântico: Tentando atribuir valor de tipo inválido/erro para '%s'.\n", $1);
             $$ = NULL;
             // YYERROR já deve ter sido chamado
        } else if (strcmp(sim->tipo, $3->tipoDado) != 0) {
             // Permitir atribuição int -> float
             if (!(strcmp(sim->tipo, "float") == 0 && strcmp($3->tipoDado, "int") == 0)) {
                fprintf(stderr, "Erro Semântico: Atribuição incompatível. Variável '%s' é '%s', mas expressão é '%s'.\n", $1, sim->tipo, $3->tipoDado);
                $$ = NULL;
                YYERROR;
             } else {
                 // Permitido int -> float, continua
                 // printf("[ATRIB] %s = ... (com coerção int->float)\n", $1);
                 $$ = criarNo(AST_ATRIB, sim->nome, sim->tipo, 1, $3);
             }
        } else {
            // printf("[ATRIB] %s = ...\n", $1);
            $$ = criarNo(AST_ATRIB, sim->nome, sim->tipo, 1, $3);
        }
        free($1); // Libera memória do ID
    }
    ;



print: /* Simplificado - Apenas imprime string literal por enquanto */
    ID LPAREN STRING RPAREN SEMICOLON {
        // No futuro, poderia verificar se ID é uma função 'printf'
        // printf("[PRINT] String: %s\n", $3);
        // Modificado para gerar código intermediário, não criar nó AST direto para print
        // A geração de código para PRINT será feita em codegen.c
        // Aqui, apenas validamos a estrutura (poderia ser mais complexo)
        $$ = criarNo(AST_PRINT, $3, "string", 0); // Nó temporário para info
        free($1); // free($3); // $3 é usado no nó
    }
    ;


bloco:
    LBRACE lista_comandos RBRACE { $$ = $2; } // $2 já é um AST_BLOCO
    | LBRACE RBRACE { $$ = criarNo(AST_BLOCO, NULL, NULL, 0); } /* Bloco vazio */
    ;


if_else:
    KW_IF LPAREN expr RPAREN comando {
        if (!$3 || strcmp($3->tipoDado, "erro") == 0) {
             // fprintf(stderr, "Erro Semântico: Condição do IF inválida.\n");
             $$ = NULL; // YYERROR já chamado
        } else if (strcmp($3->tipoDado, "int") != 0 && strcmp($3->tipoDado, "float") != 0) { // Condição deve ser numérica (ou booleana)
             fprintf(stderr, "Erro Semântico: Condição do IF deve ser numérica ou booleana, obteve '%s'.\n", $3->tipoDado);
             $$ = NULL; YYERROR;
        } else if (!$5 && $5 != NULL) { // Comando pode ser NULL (;) mas não inválido
             $$ = NULL; // Erro no corpo do IF
        } else {
             // printf("[IF]\n");
             $$ = criarNo(AST_IF, NULL, NULL, 2, $3, $5);
        }
    }
  | KW_IF LPAREN expr RPAREN comando KW_ELSE comando {
         if (!$3 || strcmp($3->tipoDado, "erro") == 0) {
             // fprintf(stderr, "Erro Semântico: Condição do IF-ELSE inválida.\n");
             $$ = NULL; // YYERROR já chamado
         } else if (strcmp($3->tipoDado, "int") != 0 && strcmp($3->tipoDado, "float") != 0) {
             fprintf(stderr, "Erro Semântico: Condição do IF-ELSE deve ser numérica ou booleana, obteve '%s'.\n", $3->tipoDado);
             $$ = NULL; YYERROR;
         } else if ((!$5 && $5 != NULL) || (!$7 && $7 != NULL)) { // Verifica corpos
             $$ = NULL; // Erro no corpo do IF ou ELSE
         } else {
            // printf("[IF/ELSE]\n");
            $$ = criarNo(AST_IF_ELSE, NULL, NULL, 3, $3, $5, $7);
         }
    }
    ;


while_loop:
    KW_WHILE LPAREN expr RPAREN comando {
        if (!$3 || strcmp($3->tipoDado, "erro") == 0) {
             // fprintf(stderr, "Erro Semântico: Condição do WHILE inválida.\n");
             $$ = NULL; // YYERROR já chamado
        } else if (strcmp($3->tipoDado, "int") != 0 && strcmp($3->tipoDado, "float") != 0) {
             fprintf(stderr, "Erro Semântico: Condição do WHILE deve ser numérica ou booleana, obteve '%s'.\n", $3->tipoDado);
             $$ = NULL; YYERROR;
        } else if (!$5 && $5 != NULL) {
             $$ = NULL; // Erro no corpo
        } else {
            // printf("[WHILE]\n");
            $$ = criarNo(AST_WHILE, NULL, NULL, 2, $3, $5);
        }
    }
    ;


for_loop: /* Simplificado - apenas estrutura, semântica básica */
    KW_FOR LPAREN atribuicao expr SEMICOLON atribuicao RPAREN comando {
        // Verificações semânticas mais detalhadas podem ser adicionadas aqui
        // para $3 (init), $4 (cond), $6 (incr)
        if (!$3 || !$4 || !$6 || !$8 || strcmp($4->tipoDado, "erro") == 0) {
             fprintf(stderr, "Erro Semântico: Componente inválido no FOR.\n");
             $$ = NULL; YYERROR;
        } else if (strcmp($4->tipoDado, "int") != 0 && strcmp($4->tipoDado, "float") != 0) {
             fprintf(stderr, "Erro Semântico: Condição do FOR deve ser numérica ou booleana, obteve '%s'.\n", $4->tipoDado);
             $$ = NULL; YYERROR;
        } else {
            // printf("[FOR]\n");
            $$ = criarNo(AST_FOR, NULL, NULL, 4, $3, $4, $6, $8);
        }
    }
    ;


do_while_loop:
    KW_DO comando KW_WHILE LPAREN expr RPAREN SEMICOLON {
        if (!$5 || strcmp($5->tipoDado, "erro") == 0) {
             // fprintf(stderr, "Erro Semântico: Condição do DO-WHILE inválida.\n");
             $$ = NULL; // YYERROR já chamado
        } else if (strcmp($5->tipoDado, "int") != 0 && strcmp($5->tipoDado, "float") != 0) {
             fprintf(stderr, "Erro Semântico: Condição do DO-WHILE deve ser numérica ou booleana, obteve '%s'.\n", $5->tipoDado);
             $$ = NULL; YYERROR;
        } else if (!$2 && $2 != NULL) {
             $$ = NULL; // Erro no corpo
        } else {
            // printf("[DO_WHILE]\n");
            $$ = criarNo(AST_DO_WHILE, NULL, NULL, 2, $2, $5);
        }
    }
    ;

expr:
      valor { $$ = $1; }
    | ID {
          Simbolo* sim = buscar_simbolo($1);
          if (!sim) {
            fprintf(stderr, "Erro Semântico: Variável '%s' não declarada.\n", $1);
            // Cria nó com tipo erro para propagar
            $$ = criarNo(AST_ID, $1, "erro", 0);
            YYERROR;
          } else {
             $$ = criarNo(AST_ID, $1, sim->tipo, 0);
          }
          free($1); // Libera memória do ID
      }
    | expr OP_PLUS expr {
          const char* tipo_result = check_arithmetic_types($1->tipoDado, $3->tipoDado, "+");
          if (strcmp(tipo_result, "erro") == 0) { $$ = criarNo(AST_EXPR, "+", "erro", 2, $1, $3); YYERROR; }
          else $$ = criarNo(AST_EXPR, "+", (char*)tipo_result, 2, $1, $3);
      }
    | expr OP_MINUS expr {
          const char* tipo_result = check_arithmetic_types($1->tipoDado, $3->tipoDado, "-");
          if (strcmp(tipo_result, "erro") == 0) { $$ = criarNo(AST_EXPR, "-", "erro", 2, $1, $3); YYERROR; }
          else $$ = criarNo(AST_EXPR, "-", (char*)tipo_result, 2, $1, $3);
      }
    | expr OP_MUL expr {
          const char* tipo_result = check_arithmetic_types($1->tipoDado, $3->tipoDado, "*");
          if (strcmp(tipo_result, "erro") == 0) { $$ = criarNo(AST_EXPR, "*", "erro", 2, $1, $3); YYERROR; }
          else $$ = criarNo(AST_EXPR, "*", (char*)tipo_result, 2, $1, $3);
      }
    | expr OP_DIV expr {
          // Adicionar verificação de divisão por zero na geração de código
          const char* tipo_result = check_arithmetic_types($1->tipoDado, $3->tipoDado, "/");
          if (strcmp(tipo_result, "erro") == 0) { $$ = criarNo(AST_EXPR, "/", "erro", 2, $1, $3); YYERROR; }
          else $$ = criarNo(AST_EXPR, "/", (char*)tipo_result, 2, $1, $3);
      }
    | expr OP_MOD expr {
          const char* tipo_result = check_arithmetic_types($1->tipoDado, $3->tipoDado, "%");
          if (strcmp(tipo_result, "erro") == 0) { $$ = criarNo(AST_EXPR, "%", "erro", 2, $1, $3); YYERROR; }
          else $$ = criarNo(AST_EXPR, "%", (char*)tipo_result, 2, $1, $3);
      }
    | expr OP_EQ expr {
          const char* tipo_result = check_comparison_logical_types($1->tipoDado, $3->tipoDado, "==");
          if (strcmp(tipo_result, "erro") == 0) { $$ = criarNo(AST_EXPR, "==", "erro", 2, $1, $3); YYERROR; }
          else $$ = criarNo(AST_EXPR, "==", (char*)tipo_result, 2, $1, $3);
      }
    | expr OP_NEQ expr {
          const char* tipo_result = check_comparison_logical_types($1->tipoDado, $3->tipoDado, "!=");
          if (strcmp(tipo_result, "erro") == 0) { $$ = criarNo(AST_EXPR, "!=", "erro", 2, $1, $3); YYERROR; }
          else $$ = criarNo(AST_EXPR, "!=", (char*)tipo_result, 2, $1, $3);
      }
    | expr OP_LT expr {
          const char* tipo_result = check_comparison_logical_types($1->tipoDado, $3->tipoDado, "<");
          if (strcmp(tipo_result, "erro") == 0) { $$ = criarNo(AST_EXPR, "<", "erro", 2, $1, $3); YYERROR; }
          else $$ = criarNo(AST_EXPR, "<", (char*)tipo_result, 2, $1, $3);
      }
    | expr OP_GT expr {
          const char* tipo_result = check_comparison_logical_types($1->tipoDado, $3->tipoDado, ">");
          if (strcmp(tipo_result, "erro") == 0) { $$ = criarNo(AST_EXPR, ">", "erro", 2, $1, $3); YYERROR; }
          else $$ = criarNo(AST_EXPR, ">", (char*)tipo_result, 2, $1, $3);
      }
    | expr OP_LEQ expr {
          const char* tipo_result = check_comparison_logical_types($1->tipoDado, $3->tipoDado, "<=");
          if (strcmp(tipo_result, "erro") == 0) { $$ = criarNo(AST_EXPR, "<=", "erro", 2, $1, $3); YYERROR; }
          else $$ = criarNo(AST_EXPR, "<=", (char*)tipo_result, 2, $1, $3);
      }
    | expr OP_GEQ expr {
          const char* tipo_result = check_comparison_logical_types($1->tipoDado, $3->tipoDado, ">=");
          if (strcmp(tipo_result, "erro") == 0) { $$ = criarNo(AST_EXPR, ">=", "erro", 2, $1, $3); YYERROR; }
          else $$ = criarNo(AST_EXPR, ">=", (char*)tipo_result, 2, $1, $3);
      }
    | expr OP_AND expr { // &&
          const char* tipo_result = check_comparison_logical_types($1->tipoDado, $3->tipoDado, "&&");
          if (strcmp(tipo_result, "erro") == 0) { $$ = criarNo(AST_EXPR, "&&", "erro", 2, $1, $3); YYERROR; }
          else $$ = criarNo(AST_EXPR, "&&", (char*)tipo_result, 2, $1, $3);
      }
    | expr OP_OR expr { // ||
          const char* tipo_result = check_comparison_logical_types($1->tipoDado, $3->tipoDado, "||");
          if (strcmp(tipo_result, "erro") == 0) { $$ = criarNo(AST_EXPR, "||", "erro", 2, $1, $3); YYERROR; }
          else $$ = criarNo(AST_EXPR, "||", (char*)tipo_result, 2, $1, $3);
      }
    | OP_NOT expr { // !
          if (!$2 || strcmp($2->tipoDado, "erro") == 0) { 
              $$ = criarNo(AST_EXPR, "!", "erro", 1, $2); // Propaga erro
              // YYERROR já chamado
          } else if (strcmp($2->tipoDado, "int") != 0 && strcmp($2->tipoDado, "float") != 0) {
               fprintf(stderr, "Erro Semântico: Operador '!' requer operando numérico ou booleano, obteve '%s'.\n", $2->tipoDado);
               $$ = criarNo(AST_EXPR, "!", "erro", 1, $2);
               YYERROR;
          } else {
               $$ = criarNo(AST_EXPR, "!", "int", 1, $2); // Resultado é booleano (int)
          }
      }
    | LPAREN expr RPAREN { $$ = $2; }
    ;

valor:
      NUMBER   {
          // Determina se é int ou float (simplificado: assume int se não houver '.')
          char* tipo = (strchr($1, '.') == NULL) ? "int" : "float";
          $$ = criarNo(AST_NUM, $1, tipo, 0);
          // free($1); // Não liberar $1 aqui, pois é usado em criarNo (strdup feito lá)
      }
    | CHAR     { $$ = criarNo(AST_CHAR, $1, "char", 0); /* free($1); */ }
    | STRING   { $$ = criarNo(AST_STRING, $1, "string", 0); /* free($1); */ }
    ;

tipo:
    KW_INT     { $$ = "int"; }
  | KW_FLOAT   { $$ = "float"; }
  | KW_CHAR    { $$ = "char"; }
  | KW_DOUBLE  { $$ = "double"; } /* Tratado como float por simplicidade */
  | KW_VOID    { $$ = "void"; }
    ;

%%

// Declaração externa para yylineno (se o lexer o fornecer)
extern int yylineno;

void yyerror(const char *s) {
    // Adiciona número da linha ao erro
    fprintf(stderr, "Erro (Linha %d): %s\n", yylineno, s);
}

// Função para liberar a AST recursivamente
void freeAST(NoAST* no) {
    if (!no) return;
    for (int i = 0; i < no->n_filhos; i++) {
        freeAST(no->filhos[i]);
    }
    if (no->valor) free(no->valor);
    if (no->tipoDado) free(no->tipoDado);
    if (no->filhos) free(no->filhos);
    free(no);
}


// Função principal - Modificada para chamar geração de código
int main(void) {
    int parse_result;
    inicializar_tabela(); // Garante que a tabela está vazia
    printf("Iniciando análise léxica/sintática/semântica...\n");
    
    parse_result = yyparse(); // yyparse retorna 0 em sucesso, 1 em erro sintático, 2 em falta de memória

    if (parse_result == 0 && raizAST != NULL) { 
        printf("\nAnálise Sintática/Semântica concluída com sucesso.\n");
        
        // Imprimir Tabela de Símbolos e AST (opcional, para debug)
        // printf("\n---- Tabela de Símbolos Final ----\n");
        // imprimirTabela();
        // printf("\n---- Árvore Sintática Abstrata (AST) ----\n");
        // imprimirAST(raizAST, 0);

        // Gerar Código Intermediário
        printf("\nIniciando geração de código intermediário...\n");
        gerarCodigo(raizAST); // Percorre a AST e preenche a lista de quádruplas
        imprimirCodigoIntermediario(); // Imprime as quádruplas geradas
        liberarCodigoIntermediario(); // Libera memória do código gerado

    } else {
        printf("\nAnálise falhou. Erros de sintaxe ou semântica encontrados.\n");
        if (parse_result == 1) {
             fprintf(stderr, "Erro: Falha na análise sintática.\n");
        } else if (parse_result == 2) {
             fprintf(stderr, "Erro: Memória insuficiente durante a análise.\n");
        }
    }

    liberar_tabela(); // Liberar memória da tabela de símbolos
    if (raizAST) {
        // freeAST(raizAST); // Liberar memória da AST
        // TODO: Implementar freeAST corretamente
    }
    
    return (parse_result == 0 && raizAST != NULL) ? 0 : 1; // Retorna 0 em sucesso, 1 em erro
}


