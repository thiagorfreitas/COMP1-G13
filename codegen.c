#include "codegen.h"
#include "tabela.h" // Para buscar informações de variáveis se necessário
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- Variáveis Globais --- 

Quadrupla* code_head = NULL; // Cabeça da lista de quádruplas
Quadrupla* code_tail = NULL; // Cauda da lista de quádruplas
int temp_count = 0;          // Contador para variáveis temporárias
int label_count = 0;         // Contador para rótulos

// --- Funções de Criação de Endereços ---

Endereco criarEnderecoVazio() {
    Endereco end;
    end.tipo = ADDR_EMPTY;
    return end;
}

Endereco criarEnderecoVar(const char* nome) {
    Endereco end;
    end.tipo = ADDR_VAR;
    end.val.varName = strdup(nome);
    return end;
}

// Cria e retorna um novo endereço temporário (ex: t0, t1, ...)
Endereco criarEnderecoTemp() {
    Endereco end;
    end.tipo = ADDR_TEMP;
    end.val.tempId = temp_count++;
    return end;
}

Endereco criarEnderecoConstInt(int valor) {
    Endereco end;
    end.tipo = ADDR_CONST_INT;
    end.val.constInt = valor;
    return end;
}

Endereco criarEnderecoConstFlt(float valor) {
    Endereco end;
    end.tipo = ADDR_CONST_FLT;
    end.val.constFlt = valor;
    return end;
}

// Cria e retorna um novo endereço de label (ex: L0, L1, ...)
Endereco criarEnderecoLabel(const char* nome_base) {
    Endereco end;
    end.tipo = ADDR_LABEL;
    char label_name[20]; // Buffer para nome do label
    sprintf(label_name, "%s%d", nome_base ? nome_base : "L", label_count++);
    end.val.varName = strdup(label_name);
    return end;
}

Endereco criarEnderecoString(const char* str) {
    Endereco end;
    end.tipo = ADDR_CONST_STR;
    end.val.varName = strdup(str);
    return end;
}

// Libera memória alocada por um Endereco (se aplicável)
void liberarEndereco(Endereco end) {
    if (end.tipo == ADDR_VAR || end.tipo == ADDR_LABEL || end.tipo == ADDR_CONST_STR) {
        // Verifica se o ponteiro não é nulo antes de liberar
        if (end.val.varName) {
           free(end.val.varName);
           // Opcional: setar para NULL para evitar double free acidental
           // end.val.varName = NULL; 
        }
    }
}

// --- Função para Emitir Quádruplas ---

// Modificado para usar OpCodeCG
void emitir(OpCodeCG op, Endereco arg1, Endereco arg2, Endereco resultado) {
    Quadrupla* nova = (Quadrupla*)malloc(sizeof(Quadrupla));
    if (!nova) {
        fprintf(stderr, "Erro: Falha ao alocar memória para quádrupla.\n");
        exit(EXIT_FAILURE);
    }
    nova->op = op;
    nova->arg1 = arg1;
    nova->arg2 = arg2;
    nova->resultado = resultado;
    nova->next = NULL;

    if (code_head == NULL) {
        code_head = nova;
        code_tail = nova;
    } else {
        code_tail->next = nova;
        code_tail = nova;
    }
}

// --- Função Principal de Geração de Código (Implementação Inicial/Parcial) ---

// Esta função percorrerá a AST e chamará 'emitir' para gerar o código.
// Retorna o Endereco (geralmente temporário) onde o resultado da sub-árvore está.
Endereco gerarCodigo(NoAST* no) {
    if (!no) return criarEnderecoVazio();

    // Declarações movidas para o início da função ou dentro dos cases específicos
    Endereco end1, end2, endRes, labelTrue, labelFalse, labelNext, labelBegin;

    switch (no->tipo) {
        case AST_BLOCO:
            for (int i = 0; i < no->n_filhos; i++) {
                Endereco tempEnd = gerarCodigo(no->filhos[i]);
                // Liberar endereços temporários retornados por comandos se não forem usados
                // if (tempEnd.tipo == ADDR_TEMP) liberarEndereco(tempEnd); 
            }
            return criarEnderecoVazio(); // Blocos não retornam valor diretamente

        case AST_DECL:
            // Não gera código, informação está na tabela de símbolos.
            return criarEnderecoVazio();

        case AST_ATRIB:
            end1 = criarEnderecoVar(no->valor); // Nome da variável (L-value)
            end2 = gerarCodigo(no->filhos[0]); // Gera código para a expressão (R-value)
            if (end2.tipo == ADDR_EMPTY || (end2.tipo == ADDR_VAR && end2.val.varName && strcmp(end2.val.varName, "erro") == 0)) {
                 // Se a expressão resultou em erro, não gera atribuição
                 liberarEndereco(end1);
                 return criarEnderecoVazio(); // Propaga erro
            }
            emitir(CG_ASSIGN, end2, criarEnderecoVazio(), end1);
            // Não liberar end2 aqui se for uma variável ou constante
            // liberarEndereco(end1); // end1 é o L-value, não liberar
            return end1; // Retorna o endereço da variável atribuída (pode ser útil)

        case AST_NUM:
            if (strcmp(no->tipoDado, "int") == 0) {
                return criarEnderecoConstInt(atoi(no->valor));
            } else { // float ou double
                return criarEnderecoConstFlt(atof(no->valor));
            }

        case AST_CHAR:
             return criarEnderecoConstInt(no->valor[1]); // Trata char como int

        case AST_STRING: // Usado para labels ou nomes, não como valor direto geralmente
             return criarEnderecoString(no->valor);

        case AST_ID:
            // Retorna o endereço da variável
            return criarEnderecoVar(no->valor);

        case AST_EXPR:
            end1 = gerarCodigo(no->filhos[0]);
            if (end1.tipo == ADDR_EMPTY) return criarEnderecoVazio(); // Propaga erro

            if (no->n_filhos == 1) { // Operador unário
                if (strcmp(no->valor, "-") == 0) {
                    endRes = criarEnderecoTemp();
                    emitir(CG_UMINUS, end1, criarEnderecoVazio(), endRes);
                    // liberarEndereco(end1); // Não liberar se for var/const
                    return endRes;
                }
                // TODO: Adicionar outros unários ( !, ~, ++, --)
                 fprintf(stderr, "Aviso: Geração de código para operador unário '%s' não implementada.\n", no->valor);
                 return criarEnderecoVazio(); // Retorna erro para unários não tratados
            } else { // Operador binário
                end2 = gerarCodigo(no->filhos[1]);
                if (end2.tipo == ADDR_EMPTY) {
                    // liberarEndereco(end1); // Não liberar se for var/const
                    return criarEnderecoVazio(); // Propaga erro
                }

                endRes = criarEnderecoTemp();
                OpCodeCG opCode; // Usa OpCodeCG
                int is_comparison = 0;

                // Mapeia string do operador para OpCodeCG
                if (strcmp(no->valor, "+") == 0) opCode = CG_ADD;
                else if (strcmp(no->valor, "-") == 0) opCode = CG_SUB;
                else if (strcmp(no->valor, "*") == 0) opCode = CG_MUL;
                else if (strcmp(no->valor, "/") == 0) opCode = CG_DIV;
                else if (strcmp(no->valor, "%") == 0) opCode = CG_MOD;
                else if (strcmp(no->valor, "==") == 0) { opCode = CG_IF_EQ; is_comparison = 1; }
                else if (strcmp(no->valor, "!=") == 0) { opCode = CG_IF_NEQ; is_comparison = 1; }
                else if (strcmp(no->valor, "<") == 0)  { opCode = CG_IF_LT; is_comparison = 1; }
                else if (strcmp(no->valor, ">") == 0)  { opCode = CG_IF_GT; is_comparison = 1; }
                else if (strcmp(no->valor, "<=") == 0) { opCode = CG_IF_LEQ; is_comparison = 1; }
                else if (strcmp(no->valor, ">=") == 0) { opCode = CG_IF_GEQ; is_comparison = 1; }
                // TODO: Tratar && e || com short-circuiting
                else {
                    fprintf(stderr, "Aviso: Operador de expressão binária '%s' não suportado na geração de código.\n", no->valor);
                    // liberarEndereco(end1);
                    // liberarEndereco(end2);
                    liberarEndereco(endRes); // Libera o temporário não usado
                    return criarEnderecoVazio(); // Retorna erro
                }

                // Se for operador aritmético, gera cálculo direto
                if (!is_comparison) {
                     emitir(opCode, end1, end2, endRes);
                } else {
                    // Para operadores relacionais, a geração é integrada com IF/WHILE.
                    // Retornamos vazio aqui, pois a lógica de salto é feita no nó pai (IF/WHILE).
                    liberarEndereco(endRes); // Libera o temporário não usado neste caso
                    return criarEnderecoVazio(); 
                }

                // liberarEndereco(end1); // Não liberar se var/const
                // liberarEndereco(end2);
                return endRes;
            }
            // break; // Removido pois todos os caminhos retornam

        case AST_IF:
            // end1 = gerarCodigo(no->filhos[0]); // Condição - Não gera valor direto
            labelFalse = criarEnderecoLabel("L_IF_FALSE");

            // Gerar código para a condição, que emitirá o salto apropriado
            // Precisamos passar o label de destino para a geração da condição.
            // Modificação necessária em gerarCodigo ou estrutura auxiliar.
            // Abordagem atual (regerar operandos): 
            if (no->filhos[0]->tipo == AST_EXPR && no->filhos[0]->n_filhos == 2) {
                 Endereco cond_op1 = gerarCodigo(no->filhos[0]->filhos[0]);
                 Endereco cond_op2 = gerarCodigo(no->filhos[0]->filhos[1]);
                 OpCodeCG cond_opCode = CG_IF_EQ; // Default
                 const char* opStr = no->filhos[0]->valor;
                 int inverted = 1; // Salta se a condição for FALSA

                 if (strcmp(opStr, "==") == 0) cond_opCode = CG_IF_NEQ; // Salta se != (falso)
                 else if (strcmp(opStr, "!=") == 0) cond_opCode = CG_IF_EQ;  // Salta se == (falso)
                 else if (strcmp(opStr, "<") == 0)  cond_opCode = CG_IF_GEQ; // Salta se >= (falso)
                 else if (strcmp(opStr, ">") == 0)  cond_opCode = CG_IF_LEQ; // Salta se <= (falso)
                 else if (strcmp(opStr, "<=") == 0) cond_opCode = CG_IF_GT;  // Salta se > (falso)
                 else if (strcmp(opStr, ">=") == 0) cond_opCode = CG_IF_LT;  // Salta se < (falso)
                 else { 
                     fprintf(stderr, "Erro: Operador relacional inválido '%s' em IF.\n", opStr);
                     inverted = 0; // Não emite salto se operador for inválido
                 }
                 
                 if (inverted) emitir(cond_opCode, cond_op1, cond_op2, labelFalse); // Salta para labelFalse se condição FALSA
                 // liberarEndereco(cond_op1);
                 // liberarEndereco(cond_op2);
            } else {
                 // Se a condição for uma variável ou valor simples, compara com 0
                 end1 = gerarCodigo(no->filhos[0]);
                 emitir(CG_IF_EQ, end1, criarEnderecoConstInt(0), labelFalse); // Salta se end1 == 0
                 // liberarEndereco(end1);
            }

            gerarCodigo(no->filhos[1]); // Corpo do IF (bloco then)

            emitir(CG_LABEL, labelFalse, criarEnderecoVazio(), criarEnderecoVazio()); // Define o label de saída
            // liberarEndereco(labelFalse); // Libera nome do label
            return criarEnderecoVazio();

        case AST_IF_ELSE:
            labelFalse = criarEnderecoLabel("L_ELSE");
            labelNext = criarEnderecoLabel("L_IF_END");
            // end1 = gerarCodigo(no->filhos[0]); // Condição - Não gera valor direto

            // Gerar código para a condição, saltando para ELSE se for FALSA
            if (no->filhos[0]->tipo == AST_EXPR && no->filhos[0]->n_filhos == 2) {
                 Endereco cond_op1 = gerarCodigo(no->filhos[0]->filhos[0]);
                 Endereco cond_op2 = gerarCodigo(no->filhos[0]->filhos[1]);
                 OpCodeCG cond_opCode = CG_IF_EQ; // Default
                 const char* opStr = no->filhos[0]->valor;
                 int inverted = 1;

                 if (strcmp(opStr, "==") == 0) cond_opCode = CG_IF_NEQ; 
                 else if (strcmp(opStr, "!=") == 0) cond_opCode = CG_IF_EQ;  
                 else if (strcmp(opStr, "<") == 0)  cond_opCode = CG_IF_GEQ; 
                 else if (strcmp(opStr, ">") == 0)  cond_opCode = CG_IF_LEQ; 
                 else if (strcmp(opStr, "<=") == 0) cond_opCode = CG_IF_GT;  
                 else if (strcmp(opStr, ">=") == 0) cond_opCode = CG_IF_LT;  
                 else { 
                     fprintf(stderr, "Erro: Operador relacional inválido '%s' em IF-ELSE.\n", opStr);
                     inverted = 0; 
                 }
                 
                 if (inverted) emitir(cond_opCode, cond_op1, cond_op2, labelFalse); // Salta para ELSE se condição FALSA
                 // liberarEndereco(cond_op1);
                 // liberarEndereco(cond_op2);
            } else {
                 end1 = gerarCodigo(no->filhos[0]);
                 emitir(CG_IF_EQ, end1, criarEnderecoConstInt(0), labelFalse); // Salta para ELSE se end1 == 0
                 // liberarEndereco(end1);
            }

            gerarCodigo(no->filhos[1]); // Corpo do IF (bloco then)
            emitir(CG_GOTO, labelNext, criarEnderecoVazio(), criarEnderecoVazio()); // Salta para o fim após o THEN

            emitir(CG_LABEL, labelFalse, criarEnderecoVazio(), criarEnderecoVazio()); // Define o label do ELSE
            // liberarEndereco(labelFalse);
            gerarCodigo(no->filhos[2]); // Corpo do ELSE

            emitir(CG_LABEL, labelNext, criarEnderecoVazio(), criarEnderecoVazio()); // Define o label de fim
            // liberarEndereco(labelNext);
            return criarEnderecoVazio();

        case AST_WHILE:
            labelBegin = criarEnderecoLabel("L_WHILE_BEGIN"); 
            labelFalse = criarEnderecoLabel("L_WHILE_END");

            emitir(CG_LABEL, labelBegin, criarEnderecoVazio(), criarEnderecoVazio()); // Label de início do loop

            // Gerar código para a condição, saltando para FIM se for FALSA
             if (no->filhos[0]->tipo == AST_EXPR && no->filhos[0]->n_filhos == 2) {
                 Endereco cond_op1 = gerarCodigo(no->filhos[0]->filhos[0]);
                 Endereco cond_op2 = gerarCodigo(no->filhos[0]->filhos[1]);
                 OpCodeCG cond_opCode = CG_IF_EQ; // Default
                 const char* opStr = no->filhos[0]->valor;
                 int inverted = 1;

                 if (strcmp(opStr, "==") == 0) cond_opCode = CG_IF_NEQ; 
                 else if (strcmp(opStr, "!=") == 0) cond_opCode = CG_IF_EQ;  
                 else if (strcmp(opStr, "<") == 0)  cond_opCode = CG_IF_GEQ; 
                 else if (strcmp(opStr, ">") == 0)  cond_opCode = CG_IF_LEQ; 
                 else if (strcmp(opStr, "<=") == 0) cond_opCode = CG_IF_GT;  
                 else if (strcmp(opStr, ">=") == 0) cond_opCode = CG_IF_LT;  
                 else { 
                     fprintf(stderr, "Erro: Operador relacional inválido '%s' em WHILE.\n", opStr);
                     inverted = 0; 
                 }
                 
                 if (inverted) emitir(cond_opCode, cond_op1, cond_op2, labelFalse); // Salta para FIM se condição FALSA
                 // liberarEndereco(cond_op1);
                 // liberarEndereco(cond_op2);
            } else {
                 end1 = gerarCodigo(no->filhos[0]);
                 emitir(CG_IF_EQ, end1, criarEnderecoConstInt(0), labelFalse); // Salta para FIM se end1 == 0
                 // liberarEndereco(end1);
            }

            gerarCodigo(no->filhos[1]); // Corpo do WHILE
            emitir(CG_GOTO, labelBegin, criarEnderecoVazio(), criarEnderecoVazio()); // Volta para o início

            emitir(CG_LABEL, labelFalse, criarEnderecoVazio(), criarEnderecoVazio()); // Label de fim do loop
            // liberarEndereco(labelBegin);
            // liberarEndereco(labelFalse);
            return criarEnderecoVazio();

        // --- Casos Adicionais (FOR, DO_WHILE, PRINT, etc.) --- 
        // Precisam ser implementados de forma similar

        case AST_PRINT:
            // Simplificado: Assume que o valor é uma string literal passada no nó
            if (no->valor) { // Se o nó PRINT tem um valor (string literal)
                 end1 = criarEnderecoString(no->valor);
                 emitir(CG_PRINT, end1, criarEnderecoVazio(), criarEnderecoVazio());
                 // liberarEndereco(end1); // String é constante, não liberar aqui
            } else {
                 // Se não tem valor, talvez devesse imprimir um filho? (não implementado)
                 fprintf(stderr, "Aviso: AST_PRINT sem valor direto não implementado para codegen.\n");
            }
            return criarEnderecoVazio();

        default:
            fprintf(stderr, "Aviso: Geração de código não implementada para o tipo de nó AST %d\n", no->tipo);
            return criarEnderecoVazio();
    }
    // return criarEnderecoVazio(); // Removido, pois todos os cases retornam
}

// --- Funções de Impressão e Liberação ---

// Imprime um endereço de forma legível
void imprimirEndereco(Endereco end) {
    switch (end.tipo) {
        case ADDR_EMPTY:
            printf("_");
            break;
        case ADDR_VAR:
            printf("%s", end.val.varName ? end.val.varName : "(null)");
            break;
        case ADDR_TEMP:
            printf("t%d", end.val.tempId);
            break;
        case ADDR_CONST_INT:
            printf("%d", end.val.constInt);
            break;
        case ADDR_CONST_FLT:
            printf("%.2f", end.val.constFlt);
            break;
        case ADDR_LABEL:
            printf("%s", end.val.varName ? end.val.varName : "(null)");
            break;
        case ADDR_CONST_STR:
            printf("\"%s\"", end.val.varName ? end.val.varName : "(null)");
            break;
    }
}

void imprimirCodigoIntermediario() {
    printf("\n---- Código Intermediário (Quádruplas) ----\n");
    Quadrupla* atual = code_head;
    int count = 0;
    if (!atual) {
        printf("(Nenhum código gerado)\n");
    }
    while (atual) {
        printf("%3d: ", count++);
        // Usa OpCodeCG nos cases
        switch (atual->op) {
            case CG_ASSIGN: printf("ASSIGN "); imprimirEndereco(atual->resultado); printf(" = "); imprimirEndereco(atual->arg1); break;
            case CG_ADD:    printf("ADD    "); imprimirEndereco(atual->resultado); printf(" = "); imprimirEndereco(atual->arg1); printf(" + "); imprimirEndereco(atual->arg2); break;
            case CG_SUB:    printf("SUB    "); imprimirEndereco(atual->resultado); printf(" = "); imprimirEndereco(atual->arg1); printf(" - "); imprimirEndereco(atual->arg2); break;
            case CG_MUL:    printf("MUL    "); imprimirEndereco(atual->resultado); printf(" = "); imprimirEndereco(atual->arg1); printf(" * "); imprimirEndereco(atual->arg2); break;
            case CG_DIV:    printf("DIV    "); imprimirEndereco(atual->resultado); printf(" = "); imprimirEndereco(atual->arg1); printf(" / "); imprimirEndereco(atual->arg2); break;
            case CG_MOD:    printf("MOD    "); imprimirEndereco(atual->resultado); printf(" = "); imprimirEndereco(atual->arg1); printf(" %% "); imprimirEndereco(atual->arg2); break;
            case CG_UMINUS: printf("UMINUS "); imprimirEndereco(atual->resultado); printf(" = - "); imprimirEndereco(atual->arg1); break;
            case CG_LABEL:  printf("LABEL  %s:", atual->arg1.val.varName ? atual->arg1.val.varName : "(null)"); break;
            case CG_GOTO:   printf("GOTO   "); imprimirEndereco(atual->arg1); break;
            case CG_IF_EQ:  printf("IF_EQ  "); imprimirEndereco(atual->arg1); printf(" == "); imprimirEndereco(atual->arg2); printf(" GOTO "); imprimirEndereco(atual->resultado); break;
            case CG_IF_NEQ: printf("IF_NEQ "); imprimirEndereco(atual->arg1); printf(" != "); imprimirEndereco(atual->arg2); printf(" GOTO "); imprimirEndereco(atual->resultado); break;
            case CG_IF_LT:  printf("IF_LT  "); imprimirEndereco(atual->arg1); printf(" <  "); imprimirEndereco(atual->arg2); printf(" GOTO "); imprimirEndereco(atual->resultado); break;
            case CG_IF_GT:  printf("IF_GT  "); imprimirEndereco(atual->arg1); printf(" >  "); imprimirEndereco(atual->arg2); printf(" GOTO "); imprimirEndereco(atual->resultado); break;
            case CG_IF_LEQ: printf("IF_LEQ "); imprimirEndereco(atual->arg1); printf(" <= "); imprimirEndereco(atual->arg2); printf(" GOTO "); imprimirEndereco(atual->resultado); break;
            case CG_IF_GEQ: printf("IF_GEQ "); imprimirEndereco(atual->arg1); printf(" >= "); imprimirEndereco(atual->arg2); printf(" GOTO "); imprimirEndereco(atual->resultado); break;
            case CG_PRINT:  printf("PRINT  "); imprimirEndereco(atual->arg1); break;
            // Adicionar outros opcodes (CG_PARAM, CG_CALL, CG_RETURN)
            default: printf("OP_CG_%d ?", atual->op); break;
        }
        printf("\n");
        atual = atual->next;
    }
    printf("----------------------------------------\n");
}

void liberarCodigoIntermediario() {
    Quadrupla* atual = code_head;
    Quadrupla* prox;
    while (atual) {
        prox = atual->next;
        // Liberar memória dos endereços que alocaram strings
        liberarEndereco(atual->arg1);
        liberarEndereco(atual->arg2);
        liberarEndereco(atual->resultado);
        free(atual);
        atual = prox;
    }
    code_head = NULL;
    code_tail = NULL;
    temp_count = 0; // Resetar contadores
    label_count = 0;
}

