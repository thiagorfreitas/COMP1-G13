#include "ast2.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

NoAST* criarNo(TipoAST tipo, char* valor, char* tipoDado, int n_filhos, ...) {
    NoAST* no = malloc(sizeof(NoAST));
    no->tipo = tipo;
    no->valor = valor ? strdup(valor) : NULL;
    no->tipoDado = tipoDado ? strdup(tipoDado) : NULL;
    no->n_filhos = n_filhos;
    no->filhos = n_filhos > 0 ? malloc(n_filhos * sizeof(NoAST*)) : NULL;

    va_list args;
    va_start(args, n_filhos);
    for (int i = 0; i < n_filhos; i++)
        no->filhos[i] = va_arg(args, NoAST*);
    va_end(args);

    return no;
}

void imprimirAST(NoAST* no, int nivel) {
    if (!no) return;
    // Nome do tipo do nó (opcional, para deixar legível)
    static const char* nomes[] = {
        "DECL", "ATRIB", "PRINT", "IF", "IF_ELSE", "WHILE", "FOR", "DO_WHILE",
        "BLOCO", "EXPR", "ID", "NUM", "CHAR", "STRING"
    };

    for (int i = 0; i < nivel; i++) printf("  ");
    printf("%s", nomes[no->tipo]);
    if (no->valor) printf(" [%s]", no->valor);
    if (no->tipoDado) printf(" <%s>", no->tipoDado);
    printf("\n");
    for (int i = 0; i < no->n_filhos; i++)
        imprimirAST(no->filhos[i], nivel + 1);
}


