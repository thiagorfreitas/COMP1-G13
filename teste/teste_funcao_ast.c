#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

void salvarSaidaAST(NoAST* raiz, const char* arquivo) {
    FILE* fp = freopen(arquivo, "w", stdout); 
    imprimirAST(raiz, 0);
    fflush(stdout);
    fclose(fp);
    freopen("/dev/tty", "w", stdout);

int compararArquivos(const char* a, const char* b) {
    FILE *fa = fopen(a, "r");
    FILE *fb = fopen(b, "r");
    if (!fa || !fb) return 0;

    int ca, cb;
    do {
        ca = fgetc(fa);
        cb = fgetc(fb);
        if (ca != cb) {
            fclose(fa);
            fclose(fb);
            return 0;
        }
    } while (ca != EOF && cb != EOF);

    fclose(fa);
    fclose(fb);
    return 1;
}

void testeImprimirAST() {
    NoAST* filho1 = criarNo(2, "filho1", "int", 0);
    NoAST* filho2 = criarNo(3, "filho2", "float", 0);
    NoAST* raiz = criarNo(1, "raiz", "void", 2, filho1, filho2);

    salvarSaidaAST(raiz, "saida_real.txt");

    FILE* esperado = fopen("saida_esperada.txt", "w");
    fprintf(esperado, "Tipo: 1, Valor: raiz, TipoDado: void\n");
    fprintf(esperado, "  Tipo: 2, Valor: filho1, TipoDado: int\n");
    fprintf(esperado, "  Tipo: 3, Valor: filho2, TipoDado: float\n");
    fclose(esperado);

    if (compararArquivos("saida_real.txt", "saida_esperada.txt")) {
        printf(" testeImprimirAST passou!\n");
    } else {
        printf(" testeImprimirAST falhou! A saída não é igual à esperada.\n");
    }

    liberarAST(raiz);
}

int main() {
    testeImprimirAST();
    return 0;
}
