#include "codegen.h"
#include "tabela.h" // Se você tiver uma tabela de símbolos
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- Variáveis Globais ---
Quadrupla* code_head = NULL;
Quadrupla* code_tail = NULL;
int temp_count = 0;

// --- Funções Auxiliares de Memória (robustas) ---
Endereco criarEnderecoVazio() { Endereco e = {ADDR_EMPTY}; return e; }
Endereco criarEnderecoVar(const char* n) { Endereco e = {ADDR_VAR}; e.val.varName = strdup(n); return e; }
Endereco criarEnderecoTemp() { Endereco e = {ADDR_TEMP}; e.val.tempId = temp_count++; return e; }
Endereco criarEnderecoConstInt(int v) { Endereco e = {ADDR_CONST_INT}; e.val.constInt = v; return e; }
Endereco criarEnderecoConstFlt(float v) { Endereco e = {ADDR_CONST_FLT}; e.val.constFlt = v; return e; }
Endereco criarEnderecoString(const char* s) { Endereco e = {ADDR_CONST_STR}; e.val.varName = strdup(s); return e; }

void liberarEndereco(Endereco* end) {
    if (end && (end->tipo == ADDR_VAR || end->tipo == ADDR_CONST_STR)) {
        free(end->val.varName);
        end->val.varName = NULL;
    }
}

Endereco duplicarEndereco(Endereco end) {
    if (end.tipo == ADDR_VAR || end.tipo == ADDR_CONST_STR) {
        end.val.varName = strdup(end.val.varName);
    }
    return end;
}

void emitir(OpCodeCG op, Endereco arg1, Endereco arg2, Endereco resultado) {
    Quadrupla* nova = (Quadrupla*)malloc(sizeof(Quadrupla));
    nova->op = op;
    nova->arg1 = duplicarEndereco(arg1);
    nova->arg2 = duplicarEndereco(arg2);
    nova->resultado = duplicarEndereco(resultado);
    nova->next = NULL;

    if (!code_head) {
        code_head = code_tail = nova;
    } else {
        code_tail->next = nova;
        code_tail = nova;
    }
}

// --- Geração de Código Principal (HLIR) ---
Endereco gerarCodigo(NoAST* no) {
    if (!no) return criarEnderecoVazio();

    Endereco end1, end2, endRes, endCond;

    switch (no->tipo) {
        // Um bloco é uma sequência de comandos. Geramos código para cada filho.
        case AST_FUNC_DEF: {
            // A definição de uma função não gera código executável em si,
            // mas seu corpo sim. Apenas continuamos a travessia para
            // o nó filho, que é o bloco de comandos da função.
            if (no->n_filhos > 0) {
                gerarCodigo(no->filhos[0]);
            }
            return criarEnderecoVazio();
        }

        case AST_RETURN: {
            // Se for um "return valor;"
            if (no->n_filhos > 0) {
                end1 = gerarCodigo(no->filhos[0]);
                emitir(CG_RETURN, end1, criarEnderecoVazio(), criarEnderecoVazio());
                liberarEndereco(&end1);
            } else { // Se for um "return;" vazio
                emitir(CG_RETURN, criarEnderecoVazio(), criarEnderecoVazio(), criarEnderecoVazio());
            }
            return criarEnderecoVazio();
        }
        case AST_BLOCO:
            for (int i = 0; i < no->n_filhos; i++) {
                Endereco temp = gerarCodigo(no->filhos[i]);
                liberarEndereco(&temp); // Libera endereços temporários que não são usados
            }
            return criarEnderecoVazio();

        // Declarações não geram código executável, apenas entram na tabela de símbolos
        case AST_DECL:
            return criarEnderecoVazio();

        // Atribuição: gera código para a expressão e depois emite o ASSIGN
        case AST_ATRIB: {
            end1 = criarEnderecoVar(no->valor); // Endereço da variável (LHS)
            end2 = gerarCodigo(no->filhos[0]);   // Endereço do resultado da expressão (RHS)
            emitir(CG_ASSIGN, end2, criarEnderecoVazio(), end1);
            liberarEndereco(&end2);
            return end1;
        }

        // Terminais: convertem o valor do nó em um Endereço
        case AST_NUM:
            return (strcmp(no->tipoDado, "int") == 0)
                   ? criarEnderecoConstInt(atoi(no->valor))
                   : criarEnderecoConstFlt(atof(no->valor));
        case AST_ID:
            return criarEnderecoVar(no->valor);
        case AST_STRING:
            return criarEnderecoString(no->valor);

        // Expressões: o coração da geração de código para cálculos e comparações
        case AST_EXPR: {
            // Caso unário (ex: -x)
            if (no->n_filhos == 1) {
                end1 = gerarCodigo(no->filhos[0]);
                endRes = criarEnderecoTemp();
                emitir(CG_UMINUS, end1, criarEnderecoVazio(), endRes);
                liberarEndereco(&end1);
                return endRes;
            }

            // Caso binário (ex: a + b, a < b)
            end1 = gerarCodigo(no->filhos[0]);
            end2 = gerarCodigo(no->filhos[1]);
            endRes = criarEnderecoTemp();
            OpCodeCG opCode;

            // Mapeia o operador do nó para o nosso OpCodeCG
            const char* op = no->valor;
                 if (strcmp(op, "+") == 0) opCode = CG_ADD;
            else if (strcmp(op, "-") == 0) opCode = CG_SUB;
            else if (strcmp(op, "*") == 0) opCode = CG_MUL;
            else if (strcmp(op, "/") == 0) opCode = CG_DIV;
            else if (strcmp(op, "%") == 0) opCode = CG_MOD;
            else if (strcmp(op, "<") == 0) opCode = CG_LT;
            else if (strcmp(op, ">") == 0) opCode = CG_GT;
            else if (strcmp(op, "<=") == 0) opCode = CG_LTE;
            else if (strcmp(op, ">=") == 0) opCode = CG_GTE;
            else if (strcmp(op, "==") == 0) opCode = CG_EQ;
            else if (strcmp(op, "!=") == 0) opCode = CG_NEQ;
            else { /* Tratar erro de operador desconhecido */ opCode = -1; }

            // Emite a quádrupla para a operação e retorna o temporário com o resultado
            if(opCode != -1) emitir(opCode, end1, end2, endRes);
            liberarEndereco(&end1);
            liberarEndereco(&end2);
            return endRes;
        }

        // Estruturas de Controle (IF/ELSE)
        case AST_IF:
        case AST_IF_ELSE: {
            endCond = gerarCodigo(no->filhos[0]); // 1. Gera código para a condição
            emitir(CG_IF_START, endCond, criarEnderecoVazio(), criarEnderecoVazio());
            liberarEndereco(&endCond);

            emitir(CG_THEN_START, criarEnderecoVazio(), criarEnderecoVazio(), criarEnderecoVazio());
            gerarCodigo(no->filhos[1]); // 2. Gera código para o bloco THEN

            if (no->tipo == AST_IF_ELSE) {
                emitir(CG_ELSE_START, criarEnderecoVazio(), criarEnderecoVazio(), criarEnderecoVazio());
                gerarCodigo(no->filhos[2]); // 3. Gera código para o bloco ELSE
            }

            emitir(CG_IF_END, criarEnderecoVazio(), criarEnderecoVazio(), criarEnderecoVazio()); // 4. Finaliza
            return criarEnderecoVazio();
        }
        
        // Estruturas de Controle (WHILE)
        case AST_WHILE: {
            emitir(CG_WHILE_START, criarEnderecoVazio(), criarEnderecoVazio(), criarEnderecoVazio());
            emitir(CG_BODY_START, criarEnderecoVazio(), criarEnderecoVazio(), criarEnderecoVazio());
            
            // --- Início do Corpo do Loop ---
            
            // 1. Gera o código da CONDIÇÃO DENTRO do laço
            endCond = gerarCodigo(no->filhos[0]); 
            
            // 2. Emite a instrução de quebra condicional
            emitir(CG_BREAK_IF_FALSE, endCond, criarEnderecoVazio(), criarEnderecoVazio());
            liberarEndereco(&endCond);

            // 3. Gera o código do CORPO original do laço
            gerarCodigo(no->filhos[1]);
            
            // --- Fim do Corpo do Loop ---

            emitir(CG_WHILE_END, criarEnderecoVazio(), criarEnderecoVazio(), criarEnderecoVazio());
            return criarEnderecoVazio();
        }
        
        case AST_DO_WHILE: {
            emitir(CG_DO_WHILE_START, criarEnderecoVazio(), criarEnderecoVazio(), criarEnderecoVazio());
            emitir(CG_BODY_START, criarEnderecoVazio(), criarEnderecoVazio(), criarEnderecoVazio());

            // --- Início do Corpo do Loop ---

            // 1. Gera o código do CORPO do laço primeiro.
            //    (Supondo que o corpo é o filho 0 e a condição o filho 1)
            gerarCodigo(no->filhos[0]);

            // 2. Gera o código da CONDIÇÃO no final do corpo.
            endCond = gerarCodigo(no->filhos[1]);

            // 3. Emite a instrução de quebra condicional (REUTILIZAMOS o opcode!)
            emitir(CG_BREAK_IF_FALSE, endCond, criarEnderecoVazio(), criarEnderecoVazio());
            liberarEndereco(&endCond);

            // --- Fim do Corpo do Loop ---

            emitir(CG_DO_WHILE_END, criarEnderecoVazio(), criarEnderecoVazio(), criarEnderecoVazio());
            return criarEnderecoVazio();
        }

        // Chamada de Função (ex: nosso AST_PRINT)
        case AST_PRINT: {
            // 1. Empilha os parâmetros na ordem inversa
            for (int i = no->n_filhos - 1; i >= 0; i--) {
                end1 = gerarCodigo(no->filhos[i]);
                emitir(CG_PARAM, end1, criarEnderecoVazio(), criarEnderecoVazio());
                liberarEndereco(&end1);
            }
            // 2. Emite a chamada
            end1 = criarEnderecoString("printf"); // Nome da função
            end2 = criarEnderecoConstInt(no->n_filhos); // Número de parâmetros
            emitir(CG_CALL, end1, end2, criarEnderecoVazio()); // Retorno ignorado por enquanto
            liberarEndereco(&end1);
            liberarEndereco(&end2);
            return criarEnderecoVazio();
        }
        
        default:
            // Para outros tipos de nós AST que não geram código
            return criarEnderecoVazio();
    }
}

// --- Funções de Impressão e Liberação ---
void imprimirEndereco(Endereco end) {
    switch(end.tipo) {
        case ADDR_EMPTY:     printf("_"); break;
        case ADDR_VAR:       printf("%s", end.val.varName); break;
        case ADDR_TEMP:      printf("t%d", end.val.tempId); break;
        case ADDR_CONST_INT: printf("%d", end.val.constInt); break;
        case ADDR_CONST_FLT: printf("%.2f", end.val.constFlt); break;
        case ADDR_CONST_STR: printf("%s", end.val.varName); break;
        default:             printf("?"); break;
    }
}

// Função de impressão atualizada para todos os novos opcodes
void imprimirCodigoIntermediario() {
    printf("\n---- Código Intermediário (HLIR Estruturado) ----\n");
    Quadrupla* atual = code_head;
    int count = 0;
    while(atual) {
        printf("%3d: ", count++);
        
        // Helper para imprimir quádruplas no formato "res = arg1 op arg2"
        #define PRINT_OP(op_name, op_symbol) \
            printf("%-7s ", op_name); \
            imprimirEndereco(atual->resultado); printf(" = "); \
            imprimirEndereco(atual->arg1); printf(" %s ", op_symbol); \
            imprimirEndereco(atual->arg2); break
        
        switch(atual->op) {
            case CG_ASSIGN: printf("ASSIGN  "); imprimirEndereco(atual->resultado); printf(" = "); imprimirEndereco(atual->arg1); break;
            
            case CG_ADD:    PRINT_OP("ADD", "+");
            case CG_SUB:    PRINT_OP("SUB", "-");
            case CG_MUL:    PRINT_OP("MUL", "*");
            case CG_DIV:    PRINT_OP("DIV", "/");
            case CG_MOD:    PRINT_OP("MOD", "%%");
            
            case CG_LT:     PRINT_OP("LT", "<");
            case CG_GT:     PRINT_OP("GT", ">");
            case CG_LTE:    PRINT_OP("LTE", "<=");
            case CG_GTE:    PRINT_OP("GTE", ">=");
            case CG_EQ:     PRINT_OP("EQ", "==");
            case CG_NEQ:    PRINT_OP("NEQ", "!=");

            case CG_UMINUS: printf("UMINUS  "); imprimirEndereco(atual->resultado); printf(" = -"); imprimirEndereco(atual->arg1); break;
            
            case CG_PARAM:  printf("PARAM   "); imprimirEndereco(atual->arg1); break;
            case CG_CALL:   printf("CALL    "); imprimirEndereco(atual->arg1); printf(", "); imprimirEndereco(atual->arg2); break;
            
            case CG_IF_START:   printf("IF_START"); printf(" "); imprimirEndereco(atual->arg1); break; // Mude de "IF" para "IF_START"
            
            case CG_THEN_START: printf("  THEN_START"); break;
            case CG_ELSE_START: printf("  ELSE_START"); break;
            case CG_IF_END:     printf("IF_END"); break;

            case CG_WHILE_START:    printf("WHILE_START"); break;
            case CG_WHILE_COND:     printf("  WHILE_COND "); imprimirEndereco(atual->arg1); break;
            case CG_BODY_START:     printf("    BODY_START"); break;
            case CG_WHILE_END:      printf("WHILE_END"); break;
            case CG_BREAK_IF_FALSE: printf("  BREAK_IF_FALSE "); imprimirEndereco(atual->arg1); break;
            case CG_DO_WHILE_START: printf("DO_WHILE_START"); break;
            case CG_DO_WHILE_END:   printf("DO_WHILE_END"); break;
            case CG_RETURN:   printf("RETURN  "); imprimirEndereco(atual->arg1); break;
            
            default: printf("OPCODE_DESCONHECIDO (%d)", atual->op);
        }
        printf("\n");
        atual = atual->next;
    }
    printf("---------------------------------------------------\n");
}

void liberarCodigoIntermediario() {
    Quadrupla* atual = code_head;
    while(atual) {
        Quadrupla* prox = atual->next;
        liberarEndereco(&atual->arg1);
        liberarEndereco(&atual->arg2);
        liberarEndereco(&atual->resultado);
        free(atual);
        atual = prox;
    }
    code_head = NULL;
    code_tail = NULL;
    temp_count = 0; // Reseta a contagem de temporários
}