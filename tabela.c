#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tabela.h"

Simbolo* tabela[TABLE_SIZE];

// 1. Função de hash para calcular o índice a partir do nome do símbolo.
//    Este é um algoritmo de hash (djb2) comum e eficiente.
static unsigned int hash(const char* nome) {
    unsigned long hash = 5381;
    int c;

    while ((c = *nome++)) {
        // A magia do hash: hash * 33 + c
        hash = ((hash << 5) + hash) + c;
    }

    return hash % TABLE_SIZE; // Garante que o índice está dentro dos limites do nosso array
}

// 2. Atualizar a inicialização da tabela
//    Percorremos o array e definimos cada posição (bucket) como NULL.
void inicializar_tabela() {
    for (int i = 0; i < TABLE_SIZE; i++) {
        tabela[i] = NULL;
    }
}

// 3. Atualizar a libertação de memória da tabela
//    Temos de percorrer cada bucket e libertar a lista ligada que possa existir lá.
void liberar_tabela() {
    for (int i = 0; i < TABLE_SIZE; i++) {
        Simbolo* atual = tabela[i];
        while (atual) {
            Simbolo* prox = atual->prox;
            free(atual->nome);
            free(atual->tipo);
            free(atual);
            atual = prox;
        }
        tabela[i] = NULL; // Limpa o ponteiro no bucket
    }
}

// 4. Atualizar a adição de símbolos
void adicionar_simbolo(const char* nome, const char* tipo) {
    // Primeiro, calcula-se o índice para saber onde inserir
    unsigned int index = hash(nome);

    Simbolo* novo = malloc(sizeof(Simbolo));
    if (!novo) {
        fprintf(stderr, "Erro: Falha ao alocar memória para símbolo\n");
        exit(EXIT_FAILURE);
    }
    novo->nome = strdup(nome);
    novo->tipo = strdup(tipo);
    if (!novo->nome || !novo->tipo) {
         fprintf(stderr, "Erro: Falha ao duplicar string para símbolo\n");
         free(novo->nome);
         free(novo->tipo);
         free(novo);
         exit(EXIT_FAILURE);
    }

    // Adiciona o novo símbolo ao início da lista ligada nesse índice específico
    novo->prox = tabela[index];
    tabela[index] = novo;
}

// 5. Atualizar a busca de símbolos (A grande otimização!)
Simbolo* buscar_simbolo(const char* nome) {
    // Calcula-se o índice para ir diretamente ao bucket correto
    unsigned int index = hash(nome);

    // Agora, só se percorre a pequena lista ligada nesse bucket,
    // em vez de percorrer todas as variáveis do programa.
    Simbolo* atual = tabela[index];
    while (atual) {
        if (strcmp(atual->nome, nome) == 0) {
            return atual; // Encontrado!
        }
        atual = atual->prox;
    }
    return NULL; // Não encontrado
}

// 6. Atualizar a função de impressão para mostrar a estrutura da hash table
void imprimirTabela(void) {
    printf("---- Tabela de Símbolos (Hash Table) ----\n");
    for (int i = 0; i < TABLE_SIZE; i++) {
        // Só imprime buckets que não estão vazios
        if (tabela[i]) {
            printf("Bucket[%3d]:\n", i);
            Simbolo* atual = tabela[i];
            while (atual) {
                printf("  -> Nome: %-12s | Tipo: %s\n", atual->nome, atual->tipo);
                atual = atual->prox;
            }
        }
    }
    printf("-----------------------------------------\n");
}
