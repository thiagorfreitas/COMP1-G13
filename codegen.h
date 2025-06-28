#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"

// --- Estruturas para Endereços (sem alteração) ---
typedef enum {
    ADDR_EMPTY,
    ADDR_VAR,
    ADDR_TEMP,
    ADDR_CONST_INT,
    ADDR_CONST_FLT,
    ADDR_CONST_STR
} TipoEndereco;

typedef struct {
    TipoEndereco tipo;
    union {
        char* varName;
        int     tempId;
        int     constInt;
        float   constFlt;
    } val;
} Endereco;


// --- NOVOS OPCODES PARA O HLIR ---
typedef enum {
    // Atribuição
    CG_ASSIGN,      // resultado = arg1

    // Aritméticos
    CG_ADD,         // resultado = arg1 + arg2
    CG_SUB,
    CG_MUL,
    CG_DIV,
    CG_MOD,
    CG_UMINUS,      // resultado = -arg1

    // Relacionais (Booleanos)
    CG_LT,          // resultado = arg1 < arg2 (Less Than)
    CG_GT,          // resultado = arg1 > arg2 (Greater Than)
    CG_LTE,         // resultado = arg1 <= arg2 (Less Than or Equal)
    CG_GTE,         // resultado = arg1 >= arg2 (Greater Than or Equal)
    CG_EQ,          // resultado = arg1 == arg2 (Equal)
    CG_NEQ,         // resultado = arg1 != arg2 (Not Equal)

    // Chamada de Função
    CG_PARAM,       // Define um parâmetro para a próxima chamada
    CG_CALL,        // resultado = call arg1, arg2 (arg1=nome, arg2=n_params)
    
    // Controle de Fluxo Estruturado
    CG_IF_START,    // IF arg1 (arg1 = resultado da condição)
    CG_THEN_START,  // Início do bloco THEN
    CG_ELSE_START,  // Início do bloco ELSE
    CG_IF_END,      // Fim da estrutura IF/ELSE
    
    CG_WHILE_START, // Início da estrutura WHILE
    CG_WHILE_COND,  // Contém a condição do WHILE (arg1 = resultado)
    CG_BODY_START,  // Início do corpo do loop/bloco
    CG_WHILE_END,   // Fim da estrutura WHILE
    CG_BREAK_IF_FALSE,
    CG_DO_WHILE_START,
    CG_DO_WHILE_END,
    CG_RETURN

} OpCodeCG;

typedef struct Quadrupla {
    OpCodeCG op;
    Endereco arg1;
    Endereco arg2;
    Endereco resultado;
    struct Quadrupla* next;
} Quadrupla;

// --- Variáveis Globais ---
extern Quadrupla* code_head;
extern Quadrupla* code_tail;

// --- Protótipos das Funções (Implementadas em codegen.c) ---
Endereco criarEnderecoVazio();
Endereco criarEnderecoVar(const char* nome);
Endereco criarEnderecoTemp();
Endereco criarEnderecoConstInt(int valor);
Endereco criarEnderecoConstFlt(float valor);
Endereco criarEnderecoString(const char* str);
void     liberarEndereco(Endereco* end);

void emitir(OpCodeCG op, Endereco arg1, Endereco arg2, Endereco resultado);
Endereco gerarCodigo(NoAST* no);
void imprimirCodigoIntermediario();
void liberarCodigoIntermediario();

#endif // CODEGEN_H