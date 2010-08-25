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
     IDENTIFIER = 258,
     CONSTANT = 259,
     STRING = 260,
     TYPE = 261,
     DYNINST_CALL = 262,
     NUMBER = 263,
     ERROR = 264,
     SIZEOF = 265,
     TRUE = 266,
     FALSE = 267,
     PTR_OP = 268,
     INC_OP = 269,
     DEC_OP = 270,
     LEFT_OP = 271,
     RIGHT_OP = 272,
     LEFT_ASSIGN = 273,
     RIGHT_ASSIGN = 274,
     AND_ASSIGN = 275,
     XOR_ASSIGN = 276,
     OR_ASSIGN = 277,
     TYPE_NAME = 278,
     TYPEDEF = 279,
     EXTERN = 280,
     STATIC = 281,
     CHAR = 282,
     SHORT = 283,
     INT = 284,
     LONG = 285,
     SIGNED = 286,
     UNSIGNED = 287,
     FLOAT = 288,
     DOUBLE = 289,
     CONST = 290,
     VOID = 291,
     STRUCT = 292,
     UNION = 293,
     ENUM = 294,
     ELLIPSIS = 295,
     IF = 296,
     LOCAL = 297,
     PARAM = 298,
     THREAD = 299,
     GLOBAL = 300,
     MACHINE = 301,
     CASE = 302,
     DEFAULT = 303,
     SWITCH = 304,
     BREAK = 305,
     RETURN = 306,
     COMMA = 307,
     AMPERSAND = 308,
     ASTERISK = 309,
     DOT = 310,
     NOT = 311,
     OR = 312,
     AND = 313,
     NOT_EQ = 314,
     EQ = 315,
     GREATER_EQ = 316,
     LESS_EQ = 317,
     COLON = 318,
     SEMI = 319,
     END_BLOCK = 320,
     START_BLOCK = 321,
     NCLOSE = 322,
     NOPEN = 323,
     DOLLAR = 324,
     BACKTICK = 325,
     ASSIGN = 326,
     SUB_ASSIGN = 327,
     ADD_ASSIGN = 328,
     MOD_ASSIGN = 329,
     DIV_ASSIGN = 330,
     MUL_ASSIGN = 331,
     OR_OP = 332,
     AND_OP = 333,
     LOWER_THAN_ELSE = 334,
     ELSE = 335
   };
#endif
/* Tokens.  */
#define IDENTIFIER 258
#define CONSTANT 259
#define STRING 260
#define TYPE 261
#define DYNINST_CALL 262
#define NUMBER 263
#define ERROR 264
#define SIZEOF 265
#define TRUE 266
#define FALSE 267
#define PTR_OP 268
#define INC_OP 269
#define DEC_OP 270
#define LEFT_OP 271
#define RIGHT_OP 272
#define LEFT_ASSIGN 273
#define RIGHT_ASSIGN 274
#define AND_ASSIGN 275
#define XOR_ASSIGN 276
#define OR_ASSIGN 277
#define TYPE_NAME 278
#define TYPEDEF 279
#define EXTERN 280
#define STATIC 281
#define CHAR 282
#define SHORT 283
#define INT 284
#define LONG 285
#define SIGNED 286
#define UNSIGNED 287
#define FLOAT 288
#define DOUBLE 289
#define CONST 290
#define VOID 291
#define STRUCT 292
#define UNION 293
#define ENUM 294
#define ELLIPSIS 295
#define IF 296
#define LOCAL 297
#define PARAM 298
#define THREAD 299
#define GLOBAL 300
#define MACHINE 301
#define CASE 302
#define DEFAULT 303
#define SWITCH 304
#define BREAK 305
#define RETURN 306
#define COMMA 307
#define AMPERSAND 308
#define ASTERISK 309
#define DOT 310
#define NOT 311
#define OR 312
#define AND 313
#define NOT_EQ 314
#define EQ 315
#define GREATER_EQ 316
#define LESS_EQ 317
#define COLON 318
#define SEMI 319
#define END_BLOCK 320
#define START_BLOCK 321
#define NCLOSE 322
#define NOPEN 323
#define DOLLAR 324
#define BACKTICK 325
#define ASSIGN 326
#define SUB_ASSIGN 327
#define ADD_ASSIGN 328
#define MOD_ASSIGN 329
#define DIV_ASSIGN 330
#define MUL_ASSIGN 331
#define OR_OP 332
#define AND_OP 333
#define LOWER_THAN_ELSE 334
#define ELSE 335




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 44 "../src/C.y"
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
   std::vector<BPatch_snippet *> *snippetList;

   
}
/* Line 1529 of yacc.c.  */
#line 238 "dynC.tab.h"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE dynClval;

