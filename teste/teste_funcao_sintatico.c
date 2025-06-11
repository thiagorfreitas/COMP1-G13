#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

void testeCriarNoSimples() {
    NoAST* no = criarNo(1, "valor", "int", 0);
    if (no && no->tipo == 1 && strcmp(no->valor, "valor") == 0 && strcmp(no->tipoDado, "int") == 0 && no->n_filhos == 0) {
        printf(" testeCriarNoSimples passou!\n");
    } else {
        printf(" testeCriarNoSimples falhou!\n");
    }
    liberarNo(no);
}

void testeAdicionarFilho() {
    NoAST* pai = criarNo(1, "pai", "int", 0);
    NoAST* filho = criarNo(2, "filho", "float", 0);

    adicionarFilho(pai, filho);

    if (pai->n_filhos == 1 && pai->filhos[0] == filho) {
        printf("testeAdicionarFilho passou!\n");
    } else {
        printf("testeAdicionarFilho falhou!\n");
    }

    liberarAST(pai);  
}

void testeASTComFilhos() {
    NoAST* filho1 = criarNo(2, "f1", "int", 0);
    NoAST* filho2 = criarNo(3, "f2", "int", 0);
    NoAST* pai = criarNo(1, "pai", "void", 2, filho1, filho2);

    if (pai->n_filhos == 2 && pai->filhos[0] == filho1 && pai->filhos[1] == filho2) {
        printf(" testeASTComFilhos passou!\n");
    } else {
        printf(" testeASTComFilhos falhou!\n");
    }

    liberarAST(pai);
}

int main() {
    testeCriarNoSimples();
    testeAdicionarFilho();
    testeASTComFilhos();

    return 0;
}
