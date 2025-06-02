%{
#include "ast.h"
#include "parser.tab.h"
#include <string.h>
#include <stdio.h>
%}

%%

"break"       { return KW_BREAK; }
"case"        { return KW_CASE; }
"char"        { return KW_CHAR; }
"const"       { return KW_CONST; }
"continue"    { return KW_CONTINUE; }
"default"     { return KW_DEFAULT; }
"do"          { return KW_DO; }
"double"      { return KW_DOUBLE; }
"else"        { return KW_ELSE; }
"enum"        { return KW_ENUM; }
"float"       { return KW_FLOAT; }
"for"         { return KW_FOR; }
"if"          { return KW_IF; }
"int"         { return KW_INT; }
"long"        { return KW_LONG; }
"return"      { return KW_RETURN; }
"short"       { return KW_SHORT; }
"signed"      { return KW_SIGNED; }
"sizeof"      { return KW_SIZEOF; }
"static"      { return KW_STATIC; }
"switch"      { return KW_SWITCH; }
"unsigned"    { return KW_UNSIGNED; }
"void"        { return KW_VOID; }
"while"       { return KW_WHILE; }

"=="          { return OP_EQ; }
"!="          { return OP_NEQ; }
"<="          { return OP_LEQ; }
">="          { return OP_GEQ; }
"&&"          { return OP_AND; }
"||"          { return OP_OR; }
"++"          { return OP_INC; }
"--"          { return OP_DEC; }

"+"           { return OP_PLUS; }
"-"           { return OP_MINUS; }
"*"           { return OP_MUL; }
"/"           { return OP_DIV; }
"="           { return OP_ASSIGN; }
"<"           { return OP_LT; }
">"           { return OP_GT; }
"&"           { return OP_BIT_AND; }
"|"           { return OP_BIT_OR; }
"^"           { return OP_BIT_XOR; }
"~"           { return OP_BIT_NOT; }
"!"           { return OP_NOT; }

"%"           { return OP_MOD; }

";"           { return SEMICOLON; }
","           { return COMMA; }
"("           { return LPAREN; }
")"           { return RPAREN; }
"{"           { return LBRACE; }
"}"           { return RBRACE; }
"["           { return LBRACKET; }
"]"           { return RBRACKET; }

"//".*        { /* Ignora */ }

"/*"([^*]|\*+[^*/])*\*+\/    { /* Ignora */ }

\"([^\"\\]|\\.)*\" {
    yylval.str = strdup(yytext);
    return STRING;
}

\'([^\'\\]|\\.)\' {
    yylval.str = strdup(yytext);
    return CHAR;
}

[a-zA-Z_][a-zA-Z0-9_]* { 
    yylval.str = strdup(yytext); 
    return ID;
}

[0-9]+(\.[0-9]+)?([eE][-+]?[0-9]+)? {
    yylval.str = strdup(yytext);
    return NUMBER;
}

[ \t\n]+      { /* Ignora espaços */ }
\r            ; // ignora carriage return (Windows)

.             { printf("Caractere não reconhecido: %s\n", yytext); }

%%

int yywrap(void) {
    return 1;
}
