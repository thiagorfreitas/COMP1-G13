#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"

// --- Estruturas para Endereços ---

typedef enum {
    ADDR_EMPTY,    // Endereço vazio/não utilizado
    ADDR_VAR,      // Variável do código fonte (referência à tabela de símbolos)
    ADDR_TEMP,     // Variável temporária gerada
    ADDR_CONST_INT,// Constante inteira
    ADDR_CONST_FLT,// Constante float
    ADDR_CONST_STR,// Constante string (para labels, etc.)
    ADDR_LABEL     // Rótulo (label)
} TipoEndereco;

typedef struct {
    TipoEndereco tipo;
    union {
        char*   varName; // Nome da variável (ADDR_VAR) ou label/string (ADDR_CONST_STR, ADDR_LABEL)
        int     tempId;  // ID da temporária (ADDR_TEMP)
        int     constInt;// Valor da constante inteira (ADDR_CONST_INT)
        float   constFlt;// Valor da constante float (ADDR_CONST_FLT)
    } val;
} Endereco;

// --- Estrutura para Instruções (Quádruplas) ---

// Renomeado para evitar conflito com tokens do Bison (%token OP_...)
typedef enum {
    CG_ASSIGN, // res = arg1
    CG_ADD,    // res = arg1 + arg2
    CG_SUB,    // res = arg1 - arg2
    CG_MUL,    // res = arg1 * arg2
    CG_DIV,    // res = arg1 / arg2
    CG_MOD,    // res = arg1 % arg2
    CG_UMINUS, // res = -arg1 (unário)
    CG_LABEL,  // Define um label: label arg1
    CG_GOTO,   // goto arg1 (label)
    CG_IF_EQ,  // if arg1 == arg2 goto res (label)
    CG_IF_NEQ, // if arg1 != arg2 goto res (label)
    CG_IF_LT,  // if arg1 <  arg2 goto res (label)
    CG_IF_GT,  // if arg1 >  arg2 goto res (label)
    CG_IF_LEQ, // if arg1 <= arg2 goto res (label)
    CG_IF_GEQ, // if arg1 >= arg2 goto res (label)
    CG_PRINT,  // print arg1 (simplificado)
    CG_PARAM,  // Passa parâmetro arg1 para função (futuro)
    CG_CALL,   // Chama função arg1, resultado em res (futuro)
    CG_RETURN  // Retorna arg1 (futuro)
} OpCodeCG; // Renomeado para OpCodeCG

typedef struct Quadrupla {
    OpCodeCG op; // Usa o enum renomeado
    Endereco arg1;
    Endereco arg2;
    Endereco resultado;
    struct Quadrupla* next; // Para lista encadeada de instruções
} Quadrupla;

// --- Variáveis Globais (Gerenciadas em codegen.c) ---

extern Quadrupla* code_head; // Ponteiro para a primeira instrução
extern Quadrupla* code_tail; // Ponteiro para a última instrução

// --- Protótipos das Funções (Implementadas em codegen.c) ---

// Funções de criação de endereços
Endereco criarEnderecoVazio();
Endereco criarEnderecoVar(const char* nome);
Endereco criarEnderecoTemp(); // Cria e retorna um novo temporário
Endereco criarEnderecoConstInt(int valor);
Endereco criarEnderecoConstFlt(float valor);
Endereco criarEnderecoLabel(const char* nome_base); // Cria e retorna um novo label
Endereco criarEnderecoString(const char* str); // Para nomes de label, etc.
void liberarEndereco(Endereco end); // Adicionado protótipo

// Função para emitir (adicionar) uma nova quádrupla à lista
void emitir(OpCodeCG op, Endereco arg1, Endereco arg2, Endereco resultado); // Usa OpCodeCG

// Função principal de geração de código a partir da AST
// Retorna o endereço (geralmente temporário) onde o resultado da sub-árvore está
Endereco gerarCodigo(NoAST* no);

// Função para imprimir o código intermediário gerado
void imprimirCodigoIntermediario();

// Função para liberar a memória do código gerado
void liberarCodigoIntermediario();

#endif // CODEGEN_H

