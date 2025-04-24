/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_MAIN_TAB_H_INCLUDED
# define YY_YY_MAIN_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    ID = 258,                      /* ID  */
    NUMBER = 259,                  /* NUMBER  */
    STRING = 260,                  /* STRING  */
    CHAR = 261,                    /* CHAR  */
    KW_INT = 262,                  /* KW_INT  */
    KW_FLOAT = 263,                /* KW_FLOAT  */
    KW_CHAR = 264,                 /* KW_CHAR  */
    KW_DOUBLE = 265,               /* KW_DOUBLE  */
    KW_RETURN = 266,               /* KW_RETURN  */
    KW_IF = 267,                   /* KW_IF  */
    KW_ELSE = 268,                 /* KW_ELSE  */
    KW_WHILE = 269,                /* KW_WHILE  */
    KW_FOR = 270,                  /* KW_FOR  */
    KW_DO = 271,                   /* KW_DO  */
    KW_VOID = 272,                 /* KW_VOID  */
    KW_LONG = 273,                 /* KW_LONG  */
    KW_SHORT = 274,                /* KW_SHORT  */
    KW_SIGNED = 275,               /* KW_SIGNED  */
    KW_UNSIGNED = 276,             /* KW_UNSIGNED  */
    KW_BREAK = 277,                /* KW_BREAK  */
    KW_CASE = 278,                 /* KW_CASE  */
    KW_CONST = 279,                /* KW_CONST  */
    KW_CONTINUE = 280,             /* KW_CONTINUE  */
    KW_DEFAULT = 281,              /* KW_DEFAULT  */
    KW_ENUM = 282,                 /* KW_ENUM  */
    KW_STATIC = 283,               /* KW_STATIC  */
    KW_SIZEOF = 284,               /* KW_SIZEOF  */
    KW_SWITCH = 285,               /* KW_SWITCH  */
    OP_ASSIGN = 286,               /* OP_ASSIGN  */
    OP_EQ = 287,                   /* OP_EQ  */
    OP_NEQ = 288,                  /* OP_NEQ  */
    OP_LEQ = 289,                  /* OP_LEQ  */
    OP_GEQ = 290,                  /* OP_GEQ  */
    OP_AND = 291,                  /* OP_AND  */
    OP_OR = 292,                   /* OP_OR  */
    OP_INC = 293,                  /* OP_INC  */
    OP_DEC = 294,                  /* OP_DEC  */
    OP_PLUS = 295,                 /* OP_PLUS  */
    OP_MINUS = 296,                /* OP_MINUS  */
    OP_MUL = 297,                  /* OP_MUL  */
    OP_DIV = 298,                  /* OP_DIV  */
    OP_LT = 299,                   /* OP_LT  */
    OP_GT = 300,                   /* OP_GT  */
    OP_BIT_AND = 301,              /* OP_BIT_AND  */
    OP_BIT_OR = 302,               /* OP_BIT_OR  */
    OP_BIT_XOR = 303,              /* OP_BIT_XOR  */
    OP_BIT_NOT = 304,              /* OP_BIT_NOT  */
    OP_NOT = 305,                  /* OP_NOT  */
    SEMICOLON = 306,               /* SEMICOLON  */
    COMMA = 307,                   /* COMMA  */
    LPAREN = 308,                  /* LPAREN  */
    RPAREN = 309,                  /* RPAREN  */
    LBRACE = 310,                  /* LBRACE  */
    RBRACE = 311,                  /* RBRACE  */
    LBRACKET = 312,                /* LBRACKET  */
    RBRACKET = 313                 /* RBRACKET  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 10 "main.y"

    char* str;

#line 126 "main.tab.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);


#endif /* !YY_YY_MAIN_TAB_H_INCLUDED  */
