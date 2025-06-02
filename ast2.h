#ifndef AST_H
#define AST_H

typedef enum {
    AST_DECL,
    AST_ATRIB,
    AST_PRINT,
    AST_IF,
    AST_IF_ELSE,
    AST_WHILE,
    AST_FOR,
    AST_DO_WHILE,
    AST_BLOCO,
    AST_EXPR,
    AST_ID,
    AST_NUM,
    AST_CHAR,
    AST_STRING
} TipoAST;

typedef struct NoAST {
    TipoAST tipo;
    char* valor;
    char* tipoDado;   
    struct NoAST **filhos; 
    int n_filhos;
} NoAST;

NoAST* criarNo(TipoAST tipo, char* valor, char* tipoDado, int n_filhos, ...);
void imprimirAST(NoAST* no, int nivel);


#endif
