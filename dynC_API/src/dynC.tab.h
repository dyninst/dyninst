/* A Bison parser, made by GNU Bison 2.7.  */

/* Bison interface for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2012 Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

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

#ifndef YY_DYNC_DYNC_TAB_H_INCLUDED
# define YY_DYNC_DYNC_TAB_H_INCLUDED
/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int dynCdebug;
#endif

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     KNOWN_ERROR_TOK = 258,
     IDENTIFIER = 259,
     CONSTANT = 260,
     STRING = 261,
     TYPE = 262,
     NUMBER = 263,
     ERROR = 264,
     EOL = 265,
     SIZEOF = 266,
     D_TRUE = 267,
     D_FALSE = 268,
     PTR_OP = 269,
     INC_OP = 270,
     DEC_OP = 271,
     LEFT_OP = 272,
     RIGHT_OP = 273,
     LEFT_ASSIGN = 274,
     RIGHT_ASSIGN = 275,
     AND_ASSIGN = 276,
     XOR_ASSIGN = 277,
     OR_ASSIGN = 278,
     TYPE_NAME = 279,
     TYPEDEF = 280,
     EXTERN = 281,
     STATIC = 282,
     CHAR = 283,
     SHORT = 284,
     INT = 285,
     LONG = 286,
     SIGNED = 287,
     UNSIGNED = 288,
     FLOAT = 289,
     DOUBLE = 290,
     CONST = 291,
     VOID = 292,
     STRUCT = 293,
     UNION = 294,
     ENUM = 295,
     ELLIPSIS = 296,
     IF = 297,
     LOCAL = 298,
     PARAM = 299,
     GLOBAL = 300,
     FUNC = 301,
     DYNINST = 302,
     INST = 303,
     REGISTER = 304,
     NEWLINE = 305,
     CASE = 306,
     DEFAULT = 307,
     SWITCH = 308,
     RETURN = 309,
     NILL = 310,
     EOF_TOK = 311,
     COMMA = 312,
     AMPERSAND = 313,
     ASTERISK = 314,
     DOT = 315,
     NOT = 316,
     OR = 317,
     AND = 318,
     NOT_EQ = 319,
     EQ = 320,
     GREATER_EQ = 321,
     LESS_EQ = 322,
     COLON = 323,
     SEMI = 324,
     END_BLOCK = 325,
     START_BLOCK = 326,
     DOLLAR = 327,
     BACKTICK = 328,
     ASSIGN = 329,
     SUB_ASSIGN = 330,
     ADD_ASSIGN = 331,
     MOD_ASSIGN = 332,
     DIV_ASSIGN = 333,
     MUL_ASSIGN = 334,
     OR_OP = 335,
     AND_OP = 336,
     NCLOSE = 337,
     NOPEN = 338,
     LOWER_THAN_ELSE = 339,
     ELSE = 340,
     LOWER_THAN_DEREF = 341
   };
#endif


#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{
/* Line 2058 of yacc.c  */
#line 66 "../src/C.y"

   int   ival;
   long  lval;
   double dval;
   char  *sval;

   const char *context;
   
   struct VariableSpec {
      bool isConstant;
      bool isStatic; 
      bool isGlobal;
      bool isLocal;
      bool isParam;
      bool isThread;
      bool isMachineState;
      bool isMutateeScope;
      
      const char * type;
   } varSpec;

   BPatch_snippet *snippet;
   BPatch_boolExpr *boolExpr;
   BPatch_funcCallExpr *funcCall;
   BPatch_variableExpr *varExpr;
   std::vector<BPatch_snippet *> *snippetList;
   std::vector<std::pair<BPatch_snippet *, const char *> > *snippetStringListPair;
   std::pair<BPatch_snippet *, const char *> *snippetStringPair;   


/* Line 2058 of yacc.c  */
#line 174 "dynC.tab.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} YYLTYPE;
# define yyltype YYLTYPE /* obsolescent; will be withdrawn */
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE dynClval;
extern YYLTYPE dynClloc;
#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int dynCparse (void *YYPARSE_PARAM);
#else
int dynCparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int dynCparse (void);
#else
int dynCparse ();
#endif
#endif /* ! YYPARSE_PARAM */

#endif /* !YY_DYNC_DYNC_TAB_H_INCLUDED  */
