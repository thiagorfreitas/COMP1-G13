#ifndef TABELA_H
#define TABELA_H

// 1. Definir o tamanho da nossa tabela de hash. Um número primo é geralmente uma boa escolha.
#define TABLE_SIZE 211

typedef struct Simbolo {
    char* nome;
    char* tipo;
    struct Simbolo* prox;
} Simbolo;

// 2. A tabela agora é um array de ponteiros para Símbolos. Cada posição é um "bucket".
extern Simbolo* tabela[TABLE_SIZE];

// As declarações das funções permanecem as mesmas
void inicializar_tabela();
void liberar_tabela();
void adicionar_simbolo(const char* nome, const char* tipo);
Simbolo* buscar_simbolo(const char* nome);
void imprimirTabela();

#endif
