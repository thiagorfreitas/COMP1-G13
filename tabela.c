#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tabela.h"

Simbolo* tabela = NULL;

void adicionar_simbolo(const char* nome, const char* tipo) {
    Simbolo* novo = malloc(sizeof(Simbolo));
    novo->nome = strdup(nome);
    novo->tipo = strdup(tipo);
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
    while (atual) {
        printf("Nome: %-12s | Tipo: %s\n", atual->nome, atual->tipo);
        atual = atual->prox;
    }
    printf("----------------------------\n");
}

