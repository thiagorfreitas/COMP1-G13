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

// --- Função Auxiliar para Gerar Código de Condição --- 
// Gera o código para uma expressão condicional e emite o salto apropriado
// Retorna 1 se sucesso, 0 se erro
int gerarCodigoCondicao(NoAST* condNode, Endereco labelDestino, int saltarSeFalso) {
    if (!condNode) return 0;

    // Caso 1: Condição é uma expressão relacional (ex: a < b)
    if (condNode->tipo == AST_EXPR && condNode->n_filhos == 2 && 
        (strcmp(condNode->valor, "==") == 0 || strcmp(condNode->valor, "!=") == 0 ||
         strcmp(condNode->valor, "<") == 0 || strcmp(condNode->valor, ">") == 0 ||
         strcmp(condNode->valor, "<=") == 0 || strcmp(condNode->valor, ">=") == 0)) 
    {
        Endereco cond_op1 = gerarCodigo(condNode->filhos[0]);
        Endereco cond_op2 = gerarCodigo(condNode->filhos[1]);
        if (cond_op1.tipo == ADDR_EMPTY || cond_op2.tipo == ADDR_EMPTY) return 0; // Erro nos operandos

        OpCodeCG cond_opCode;
        const char* opStr = condNode->valor;

        // Determina o opcode de salto. Se saltarSeFalso=1, inverte a condição.
        if (strcmp(opStr, "==") == 0) cond_opCode = saltarSeFalso ? CG_IF_NEQ : CG_IF_EQ;
        else if (strcmp(opStr, "!=") == 0) cond_opCode = saltarSeFalso ? CG_IF_EQ : CG_IF_NEQ;
        else if (strcmp(opStr, "<") == 0)  cond_opCode = saltarSeFalso ? CG_IF_GEQ : CG_IF_LT;
        else if (strcmp(opStr, ">") == 0)  cond_opCode = saltarSeFalso ? CG_IF_LEQ : CG_IF_GT;
        else if (strcmp(opStr, "<=") == 0) cond_opCode = saltarSeFalso ? CG_IF_GT : CG_IF_LEQ;
        else /* (strcmp(opStr, ">=") == 0) */ cond_opCode = saltarSeFalso ? CG_IF_LT : CG_IF_GEQ;
        
        emitir(cond_opCode, cond_op1, cond_op2, labelDestino);
        // liberarEndereco(cond_op1); // Não liberar se var/const
        // liberarEndereco(cond_op2);
        return 1;
    } 
    // TODO: Adicionar tratamento para operadores lógicos (&&, ||) com short-circuiting
    else {
        // Caso 2: Condição é uma variável ou valor simples (ex: while(x) ou if(1))
        Endereco cond_val = gerarCodigo(condNode);
        if (cond_val.tipo == ADDR_EMPTY) return 0; // Erro na expressão

        // Salta se o valor for igual a 0 (falso em C)
        OpCodeCG jumpOp = saltarSeFalso ? CG_IF_EQ : CG_IF_NEQ;
        emitir(jumpOp, cond_val, criarEnderecoConstInt(0), labelDestino);
        // liberarEndereco(cond_val); // Não liberar se var/const
        return 1;
    }
}


// --- Função Principal de Geração de Código --- 

Endereco gerarCodigo(NoAST* no) {
    if (!no) return criarEnderecoVazio();

    Endereco end1, end2, end3, end4, endRes, labelTrue, labelFalse, labelNext, labelBegin, labelCond, labelIncr, labelBody, labelEnd;

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
                // CORREÇÃO: fprintf em uma linha
                fprintf(stderr, "Aviso: Geração de código para operador unário '%s' não implementada.\n", no->valor);
                return criarEnderecoVazio(); // Retorna erro para unários não tratados
            } else { // Operador binário
                end2 = gerarCodigo(no->filhos[1]);
                if (end2.tipo == ADDR_EMPTY) {
                    // liberarEndereco(end1); // Não liberar se for var/const
                    return criarEnderecoVazio(); // Propaga erro
                }

                endRes = criarEnderecoTemp();
                OpCodeCG opCode;
                int is_comparison = 0;

                // Mapeia string do operador para OpCodeCG
                if (strcmp(no->valor, "+") == 0) opCode = CG_ADD;
                else if (strcmp(no->valor, "-") == 0) opCode = CG_SUB;
                else if (strcmp(no->valor, "*") == 0) opCode = CG_MUL;
                else if (strcmp(no->valor, "/") == 0) opCode = CG_DIV;
                else if (strcmp(no->valor, "%") == 0) opCode = CG_MOD;
                else if (strcmp(no->valor, "==") == 0 || strcmp(no->valor, "!=") == 0 ||
                         strcmp(no->valor, "<") == 0 || strcmp(no->valor, ">") == 0 ||
                         strcmp(no->valor, "<=") == 0 || strcmp(no->valor, ">=") == 0) {
                    is_comparison = 1;
                }
                // TODO: Tratar && e || com short-circuiting
                else {
                    // CORREÇÃO: fprintf em uma linha
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
                    // Para operadores relacionais, a geração é integrada com IF/WHILE/FOR.
                    // Retornamos vazio aqui, pois a lógica de salto é feita no nó pai.
                    liberarEndereco(endRes); // Libera o temporário não usado neste caso
                    return criarEnderecoVazio(); 
                }

                // liberarEndereco(end1); // Não liberar se var/const
                // liberarEndereco(end2);
                return endRes;
            }

        case AST_IF:
            labelFalse = criarEnderecoLabel("L_IF_FALSE");
            // Gera código para condição, salta para labelFalse se a condição for FALSA
            if (!gerarCodigoCondicao(no->filhos[0], labelFalse, 1)) {
                liberarEndereco(labelFalse);
                return criarEnderecoVazio(); // Erro na condição
            }
            gerarCodigo(no->filhos[1]); // Corpo do IF (bloco then)
            emitir(CG_LABEL, labelFalse, criarEnderecoVazio(), criarEnderecoVazio()); // Define o label de saída
            // liberarEndereco(labelFalse); // Libera nome do label
            return criarEnderecoVazio();

        case AST_IF_ELSE:
            labelFalse = criarEnderecoLabel("L_ELSE");
            labelNext = criarEnderecoLabel("L_IF_END");
            // Gera código para condição, salta para labelFalse (ELSE) se a condição for FALSA
            if (!gerarCodigoCondicao(no->filhos[0], labelFalse, 1)) {
                liberarEndereco(labelFalse);
                liberarEndereco(labelNext);
                return criarEnderecoVazio(); // Erro na condição
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
            // Gera código para condição, salta para labelFalse (FIM) se a condição for FALSA
            if (!gerarCodigoCondicao(no->filhos[0], labelFalse, 1)) {
                liberarEndereco(labelBegin);
                liberarEndereco(labelFalse);
                return criarEnderecoVazio(); // Erro na condição
            }
            gerarCodigo(no->filhos[1]); // Corpo do WHILE
            emitir(CG_GOTO, labelBegin, criarEnderecoVazio(), criarEnderecoVazio()); // Volta para o início
            emitir(CG_LABEL, labelFalse, criarEnderecoVazio(), criarEnderecoVazio()); // Label de fim do loop
            // liberarEndereco(labelBegin);
            // liberarEndereco(labelFalse);
            return criarEnderecoVazio();

        case AST_FOR: // Assumindo filhos: 0=init, 1=cond, 2=incr, 3=body
            if (no->n_filhos != 4) {
                 fprintf(stderr, "Erro interno: Nó AST_FOR com número incorreto de filhos (%d).\n", no->n_filhos);
                 return criarEnderecoVazio();
            }
            labelCond = criarEnderecoLabel("L_FOR_COND");
            labelIncr = criarEnderecoLabel("L_FOR_INCR"); // Opcional, mas pode ser útil
            labelBody = criarEnderecoLabel("L_FOR_BODY"); // Opcional
            labelEnd = criarEnderecoLabel("L_FOR_END");

            // 1. Inicialização
            end1 = gerarCodigo(no->filhos[0]); // Gera código para init (ex: i = 0)
            // if (end1.tipo == ADDR_TEMP) liberarEndereco(end1); // Libera se for temporário não usado

            // 2. Label da Condição
            emitir(CG_LABEL, labelCond, criarEnderecoVazio(), criarEnderecoVazio());

            // 3. Teste da Condição
            // Gera código para condição, salta para labelEnd se a condição for FALSA
            if (!gerarCodigoCondicao(no->filhos[1], labelEnd, 1)) {
                liberarEndereco(labelCond); liberarEndereco(labelIncr); 
                liberarEndereco(labelBody); liberarEndereco(labelEnd);
                return criarEnderecoVazio(); // Erro na condição
            }
            
            // (Opcional: Pular para o corpo se a condição for verdadeira)
            // emitir(CG_GOTO, labelBody, criarEnderecoVazio(), criarEnderecoVazio()); 

            // 4. (Opcional) Label do Incremento - Colocado após o corpo
            // emitir(CG_LABEL, labelIncr, criarEnderecoVazio(), criarEnderecoVazio());

            // 5. Corpo do Loop
            // emitir(CG_LABEL, labelBody, criarEnderecoVazio(), criarEnderecoVazio()); // Label do corpo
            gerarCodigo(no->filhos[3]); // Gera código para o corpo

            // 6. Incremento
            end3 = gerarCodigo(no->filhos[2]); // Gera código para incr (ex: i = i + 1)
            // if (end3.tipo == ADDR_TEMP) liberarEndereco(end3);

            // 7. Volta para a Condição
            emitir(CG_GOTO, labelCond, criarEnderecoVazio(), criarEnderecoVazio());

            // 8. Label Final
            emitir(CG_LABEL, labelEnd, criarEnderecoVazio(), criarEnderecoVazio());

            // liberarEndereco(labelCond); liberarEndereco(labelIncr); 
            // liberarEndereco(labelBody); liberarEndereco(labelEnd);
            return criarEnderecoVazio();

        case AST_DO_WHILE: // Assumindo filhos: 0=body, 1=cond
             if (no->n_filhos != 2) {
                 fprintf(stderr, "Erro interno: Nó AST_DO_WHILE com número incorreto de filhos (%d).\n", no->n_filhos);
                 return criarEnderecoVazio();
             }
            labelBegin = criarEnderecoLabel("L_DOWHILE_BEGIN");
            // labelCond = criarEnderecoLabel("L_DOWHILE_COND"); // Pode ser o mesmo que Begin

            // 1. Label de Início
            emitir(CG_LABEL, labelBegin, criarEnderecoVazio(), criarEnderecoVazio());

            // 2. Corpo do Loop
            gerarCodigo(no->filhos[0]);

            // 3. Teste da Condição
            // Gera código para condição, salta para labelBegin se a condição for VERDADEIRA
            if (!gerarCodigoCondicao(no->filhos[1], labelBegin, 0)) { // saltarSeFalso = 0
                liberarEndereco(labelBegin);
                return criarEnderecoVazio(); // Erro na condição
            }

            // liberarEndereco(labelBegin);
            return criarEnderecoVazio();

        case AST_PRINT: {
            // Se vier com 2 filhos: [0]=string literal, [1]=expressão/variável
            if (no->n_filhos == 2) {
                Endereco strEnd = gerarCodigo(no->filhos[0]);  // AST_STRING → const str
                Endereco valEnd = gerarCodigo(no->filhos[1]);  // AST_ID ou expressão
                emitir(CG_PRINT, strEnd, valEnd, criarEnderecoVazio());
            }
            // Se só 1 filho (ex.: print de expressão sem formatação)
            else if (no->n_filhos == 1) {
                Endereco arg = gerarCodigo(no->filhos[0]);
                emitir(CG_PRINT, arg, criarEnderecoVazio(), criarEnderecoVazio());
            }
            else {
                fprintf(stderr, "Aviso: PRINT com aridade %d não suportado\n", no->n_filhos);
            }
            return criarEnderecoVazio();
        }

        
        // TODO: Adicionar AST_CALL para chamadas de função genéricas
        // case AST_CALL:
            // ... gerar código para argumentos (CG_PARAM)
            // ... emitir CG_CALL
            // ... retornar endereço do resultado (se houver)

        default:
            fprintf(stderr, "Aviso: Geração de código não implementada para o tipo de nó AST %d\n", no->tipo);
            return criarEnderecoVazio();
    }
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
            // Imprime com aspas para clareza, mas o valor não as contém
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
            case CG_PRINT:  printf("PRINT  "); imprimirEndereco(atual->arg1); if (atual->arg2.tipo != ADDR_EMPTY) { printf(", "); imprimirEndereco(atual->arg2); }break;
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

