#ifndef TABELA_H
#define TABELA_H

typedef struct Simbolo {
    char* nome;
    char* tipo;
    struct Simbolo* prox;
} Simbolo;


extern Simbolo* tabela;

// Funções adicionadas para inicialização e liberação
void inicializar_tabela();
void liberar_tabela();

void adicionar_simbolo(const char* nome, const char* tipo);
Simbolo* buscar_simbolo(const char* nome);
void imprimirTabela();

#endif

