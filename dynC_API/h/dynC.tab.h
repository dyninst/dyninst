/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

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
     INF = 301,
     DYNINST = 302,
     INST = 303,
     REGISTER = 304,
     NEWLINE = 305,
     CASE = 306,
     DEFAULT = 307,
     SWITCH = 308,
     RETURN = 309,
     NILL = 310,
     COMMA = 311,
     AMPERSAND = 312,
     ASTERISK = 313,
     DOT = 314,
     NOT = 315,
     OR = 316,
     AND = 317,
     NOT_EQ = 318,
     EQ = 319,
     GREATER_EQ = 320,
     LESS_EQ = 321,
     COLON = 322,
     SEMI = 323,
     END_BLOCK = 324,
     START_BLOCK = 325,
     DOLLAR = 326,
     BACKTICK = 327,
     ASSIGN = 328,
     SUB_ASSIGN = 329,
     ADD_ASSIGN = 330,
     MOD_ASSIGN = 331,
     DIV_ASSIGN = 332,
     MUL_ASSIGN = 333,
     OR_OP = 334,
     AND_OP = 335,
     NCLOSE = 336,
     NOPEN = 337,
     LOWER_THAN_ELSE = 338,
     ELSE = 339,
     LOWER_THAN_DEREF = 340
   };
#endif
/* Tokens.  */
#define KNOWN_ERROR_TOK 258
#define IDENTIFIER 259
#define CONSTANT 260
#define STRING 261
#define TYPE 262
#define NUMBER 263
#define ERROR 264
#define EOL 265
#define SIZEOF 266
#define D_TRUE 267
#define D_FALSE 268
#define PTR_OP 269
#define INC_OP 270
#define DEC_OP 271
#define LEFT_OP 272
#define RIGHT_OP 273
#define LEFT_ASSIGN 274
#define RIGHT_ASSIGN 275
#define AND_ASSIGN 276
#define XOR_ASSIGN 277
#define OR_ASSIGN 278
#define TYPE_NAME 279
#define TYPEDEF 280
#define EXTERN 281
#define STATIC 282
#define CHAR 283
#define SHORT 284
#define INT 285
#define LONG 286
#define SIGNED 287
#define UNSIGNED 288
#define FLOAT 289
#define DOUBLE 290
#define CONST 291
#define VOID 292
#define STRUCT 293
#define UNION 294
#define ENUM 295
#define ELLIPSIS 296
#define IF 297
#define LOCAL 298
#define PARAM 299
#define GLOBAL 300
#define INF 301
#define DYNINST 302
#define INST 303
#define REGISTER 304
#define NEWLINE 305
#define CASE 306
#define DEFAULT 307
#define SWITCH 308
#define RETURN 309
#define NILL 310
#define COMMA 311
#define AMPERSAND 312
#define ASTERISK 313
#define DOT 314
#define NOT 315
#define OR 316
#define AND 317
#define NOT_EQ 318
#define EQ 319
#define GREATER_EQ 320
#define LESS_EQ 321
#define COLON 322
#define SEMI 323
#define END_BLOCK 324
#define START_BLOCK 325
#define DOLLAR 326
#define BACKTICK 327
#define ASSIGN 328
#define SUB_ASSIGN 329
#define ADD_ASSIGN 330
#define MOD_ASSIGN 331
#define DIV_ASSIGN 332
#define MUL_ASSIGN 333
#define OR_OP 334
#define AND_OP 335
#define NCLOSE 336
#define NOPEN 337
#define LOWER_THAN_ELSE 338
#define ELSE 339
#define LOWER_THAN_DEREF 340




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 70 "../src/C.y"
{
   int   ival;
   long  lval;
   double dval;
   char  *sval;

   char *context;
   
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
   std::vector<std::pair<BPatch_snippet *, char *> > *snippetStringListPair;
   std::pair<BPatch_snippet *, char *> *snippetStringPair;   
}
/* Line 1529 of yacc.c.  */
#line 249 "dynC.tab.h"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE dynClval;

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

extern YYLTYPE dynClloc;
