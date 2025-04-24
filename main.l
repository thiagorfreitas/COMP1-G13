%{
#include <stdio.h>
%}

/* Definições de padrões de token e ações */
%%

if          { printf("KW_IF\n"); }
while       { printf("KW_WHILE\n"); }
for         { printf("KW_FOR\n"); }
return      { printf("KW_RETURN\n"); }
int         { printf("KW_INT\n"); }
float       { printf("KW_FLOAT\n"); }
char        { printf("KW_CHAR\n"); }
double      { printf("KW_DOUBLE\n"); }
long        { printf("KW_LONG\n"); }
"long"([ \t]+)"long"   { printf("KW_LONG_LONG\n"); }
bool        { printf("KW_BOOL\n"); }
short       { printf("KW_SHORT\n"); }

"//".*      { /* Ignorar comentários de linha */ }
"/*"([^*]|\*+[^*/])*\*+\/    { /* Ignorar comentários de bloco */ }

[a-zA-Z_][a-zA-Z0-9_]*  { printf("IDENT(%s)\n", yytext); }

[0-9]+(\.[0-9]+)?  { printf("NUMBER(%s)\n", yytext); }

[+\-*/=;]   { printf("SYMBOL(%s)\n", yytext); }

[ \t\n]+    { /* Ignorar espaços em branco */ }

.           { printf("UNKNOWN(%s)\n", yytext); }

%%

/* Função auxiliar do Flex (quando termina o arquivo de entrada) */
int yywrap(void) {
    return 1;
}

/* Função principal: executa o analisador léxico */
int main(void) {
    yylex();
    return 0;
}