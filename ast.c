#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "ast.h"

NoAST* criarNo(TipoAST tipo, char* valor, char* tipoDado, int n_filhos, ...) {
    NoAST* no = malloc(sizeof(NoAST));
    no->tipo      = tipo;
    no->valor     = valor     ? strdup(valor)     : NULL;
    no->tipoDado  = tipoDado  ? strdup(tipoDado)  : NULL;
    no->n_filhos  = n_filhos;

    if (n_filhos > 0) {
        // calloc já zera todos os ponteiros
        no->filhos = calloc(n_filhos, sizeof(NoAST*));

        va_list args;
        va_start(args, n_filhos);
        for (int i = 0; i < n_filhos; i++) {
            no->filhos[i] = va_arg(args, NoAST*);
        }
        va_end(args);
    } else {
        no->filhos = NULL;
    }

    return no;
}

void adicionarFilho(NoAST* pai, NoAST* filho) {
    pai->filhos = realloc(pai->filhos,
                         sizeof(NoAST*) * (pai->n_filhos + 1));
    // a nova posição estará inicialmente NULL só se realloc zerou,
    // mas como estamos logo atribuindo:
    pai->filhos[pai->n_filhos] = filho;
    pai->n_filhos++;
}

void liberarNo(NoAST* no) {
    if (!no) return;
    free(no->valor);
    free(no->tipoDado);
    free(no->filhos);
    free(no);
}

void liberarAST(NoAST* raiz) {
    if (!raiz) return;
    for (int i = 0; i < raiz->n_filhos; i++) {
        liberarAST(raiz->filhos[i]);
    }
    liberarNo(raiz);
}

void imprimirAST(NoAST* no, int nivel) {
    // 1) se o nó for NULL, só imprime e volta
    if (!no) {
        printf("%*s[NULL]\n", nivel*2, "");
        return;
    }

    // 2) imprime o próprio nó com endereço e quantidade de filhos
    printf("%*sNode %p → Tipo: %d, n_filhos: %d",
           nivel*2, "", (void*)no, no->tipo, no->n_filhos);
    if (no->valor)    printf(", Valor: %s",    no->valor);
    if (no->tipoDado) printf(", TipoDado: %s", no->tipoDado);
    printf("\n");

    // 3) se não há vetor de filhos, encerra aqui
    if (!no->filhos) return;

    // 4) para cada filho, imprime o ponteiro antes de chamar recursão
    for (int i = 0; i < no->n_filhos; i++) {
        printf("%*s─ filho[%d] ptr: %p\n",
               nivel*2, "", i, (void*)no->filhos[i]);
        imprimirAST(no->filhos[i], nivel + 1);
    }
}
