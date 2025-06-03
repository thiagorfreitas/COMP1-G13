#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include <stdarg.h>

NoAST* criarNo(TipoAST tipo, char* valor, char* tipoDado, int n_filhos, ...) {
    NoAST* no = (NoAST*) malloc(sizeof(NoAST));
    no->tipo = tipo;
    no->valor = valor ? strdup(valor) : NULL;
    no->tipoDado = tipoDado ? strdup(tipoDado) : NULL;
    no->n_filhos = n_filhos;

    if (n_filhos > 0) {
        no->filhos = malloc(sizeof(NoAST*) * n_filhos);
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
    pai->filhos = realloc(pai->filhos, sizeof(NoAST*) * (pai->n_filhos + 1));
    pai->filhos[pai->n_filhos] = filho;
    pai->n_filhos++;
}

void liberarNo(NoAST* no) {
    if (!no) return;

    if (no->valor) free(no->valor);
    if (no->tipoDado) free(no->tipoDado);
    if (no->filhos) free(no->filhos);
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
    if (!no) return;

    for (int i = 0; i < nivel; i++) {
        printf("  ");
    }

    printf("Tipo: %d", no->tipo);
    if (no->valor) printf(", Valor: %s", no->valor);
    if (no->tipoDado) printf(", TipoDado: %s", no->tipoDado);
    printf("\n");

    for (int i = 0; i < no->n_filhos; i++) {
        imprimirAST(no->filhos[i], nivel + 1);
    }
}