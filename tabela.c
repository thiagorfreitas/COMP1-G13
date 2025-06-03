#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tabela.h"

Simbolo* tabela = NULL;

// Inicializa a tabela de símbolos (define como vazia)
void inicializar_tabela() {
    liberar_tabela(); // Garante que qualquer tabela anterior seja liberada
    tabela = NULL;
}

// Libera toda a memória alocada para a tabela de símbolos
void liberar_tabela() {
    Simbolo* atual = tabela;
    Simbolo* prox;
    while (atual) {
        prox = atual->prox;
        free(atual->nome);
        free(atual->tipo);
        free(atual);
        atual = prox;
    }
    tabela = NULL; // Define a tabela como vazia após liberar
}

void adicionar_simbolo(const char* nome, const char* tipo) {
    Simbolo* novo = malloc(sizeof(Simbolo));
    if (!novo) {
        fprintf(stderr, "Erro: Falha ao alocar memória para símbolo ");
        exit(EXIT_FAILURE);
    }
    novo->nome = strdup(nome);
    novo->tipo = strdup(tipo);
    if (!novo->nome || !novo->tipo) {
         fprintf(stderr, "Erro: Falha ao duplicar string para símbolo ");
         free(novo->nome); // Libera o que foi alocado
         free(novo->tipo);
         free(novo);
         exit(EXIT_FAILURE);
    }
    novo->prox = tabela;
    tabela = novo;
}

Simbolo* buscar_simbolo(const char* nome) {
    Simbolo* atual = tabela;
    while (atual) {
        if (strcmp(atual->nome, nome) == 0)
            return atual;
        atual = atual->prox;
    }
    return NULL;
}

void imprimirTabela(void) {
    Simbolo* atual = tabela;
    printf("---- Tabela de Símbolos ----\n");
    if (!atual) {
        printf("(Vazia)\n");
    }
    while (atual) {
        printf("Nome: %-12s | Tipo: %s\n", atual->nome, atual->tipo);
        atual = atual->prox;
    }
    printf("----------------------------\n");
}

