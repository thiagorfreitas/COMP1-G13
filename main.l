%{
#include "main.tab.h"  // Arquivo de tokens gerado pelo Bison
#include <string.h>
#include <stdio.h>
%}

%%

"int"           { return INT; }
"float"         { return FLOAT; }
"char"          { return CHAR; }
"double"        { return DOUBLE; }
"long"([ \t]+)"long"    { return LONGLONG; }
"if"            { return IF; }
"else"          { return ELSE; }
"while"         { return WHILE; }
"return"        { return RETURN; }

"//".*          { /* ignora comentários de linha */ }
"/*"([^*]|\*+[^*/])*\*+\/    { /* ignora comentários de bloco */ }

[a-zA-Z_][a-zA-Z0-9_]*  {
    yylval.str = strdup(yytext);
    return ID;
}

[0-9]+(\.[0-9]+)? {
    yylval.str = strdup(yytext);
    return NUMBER;
}

"=="            { return EQ; }
"="             { return ASSIGN; }
";"             { return SEMI; }
"("             { return LPAREN; }
")"             { return RPAREN; }
"{"             { return LBRACE; }
"}"             { return RBRACE; }
","             { return COMMA; }
[+\-*/]         { return OP; }

[ \t\n]+        ;  // ignora espaços

.               { printf("Token desconhecido: %s\n", yytext); }

%%

int yywrap(void) {
    return 1;
}
