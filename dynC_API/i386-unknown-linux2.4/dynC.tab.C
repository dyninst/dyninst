/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.3"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0

/* Substitute the variable and function names.  */
#define yyparse dynCparse
#define yylex   dynClex
#define yyerror dynCerror
#define yylval  dynClval
#define yychar  dynCchar
#define yydebug dynCdebug
#define yynerrs dynCnerrs


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




/* Copy the first part of user declarations.  */
#line 1 "../src/C.y"

/*                                                                                 
 * Yacc will define token IF, conflicting with dyn_regs.h                          
 * This undef is safe b/c yacc also creates an enum IF                             
 * which serves the same purpose.                                                  
 */
#undef IF
#undef RETURN


#include <stdio.h>
#include <vector>
#include <string>
#include "BPatch_image.h"
#include "BPatch_type.h"
#include "BPatch_snippet.h"
#include "dynC.h"

extern "C" {
   void yyerror(char *s); 
   void yywarn(char *s);
   int yyparse(void);
}



int yylex();

BPatch_snippet *parse_result;

extern bool interactive;
bool verbose = false;

//message file
FILE * mfile = fopen("messages.out", "w");

extern int line_num;

std::vector<BPatch_snippet *> endSnippets;



/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif

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
/* Line 193 of yacc.c.  */
#line 335 "dynC.tab.c"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 348 "dynC.tab.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int i)
#else
static int
YYID (i)
    int i;
#endif
{
  return i;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  50
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   370

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  94
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  16
/* YYNRULES -- Number of rules.  */
#define YYNRULES  68
/* YYNRULES -- Number of states.  */
#define YYNSTATES  125

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   335

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    76,     2,     2,     2,    67,     2,     2,
      88,    89,    65,    63,     2,    64,     2,    66,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      92,     2,    93,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    90,     2,    91,    77,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    68,    69,
      70,    71,    72,    73,    74,    75,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint8 yyprhs[] =
{
       0,     0,     3,     5,     8,    13,    15,    18,    21,    24,
      27,    30,    33,    35,    38,    42,    45,    48,    54,    62,
      64,    68,    74,    75,    77,    81,    83,    85,    87,    89,
      91,    95,    99,   103,   107,   111,   115,   119,   123,   125,
     127,   129,   131,   133,   135,   138,   142,   146,   150,   152,
     154,   156,   160,   162,   166,   170,   174,   178,   182,   186,
     190,   194,   198,   202,   206,   208,   211,   214,   217
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      95,     0,    -1,    98,    -1,    97,     3,    -1,    97,     3,
      78,   108,    -1,     6,    -1,    26,    97,    -1,    35,    97,
      -1,    45,    97,    -1,    42,    97,    -1,    44,    97,    -1,
      46,    97,    -1,    99,    -1,    98,    99,    -1,    73,    98,
      72,    -1,    96,    69,    -1,   108,    69,    -1,    41,    88,
     105,    89,   100,    -1,    41,    88,   105,    89,   100,    87,
     100,    -1,    99,    -1,    90,    98,    91,    -1,    75,     3,
      88,   102,    89,    -1,    -1,   103,    -1,   102,    52,   103,
      -1,   108,    -1,     5,    -1,    11,    -1,    12,    -1,   104,
      -1,   108,    92,   108,    -1,   108,    93,   108,    -1,   108,
      60,   108,    -1,   108,    62,   108,    -1,   108,    61,   108,
      -1,   108,    59,   108,    -1,   105,    58,   105,    -1,   105,
      57,   105,    -1,    45,    -1,    42,    -1,    43,    -1,    44,
      -1,    46,    -1,     3,    -1,    75,     3,    -1,   106,    75,
       3,    -1,   106,    75,     8,    -1,     3,    55,     3,    -1,
     107,    -1,     8,    -1,     7,    -1,   108,    65,   108,    -1,
     101,    -1,   107,    78,   108,    -1,   107,    80,   108,    -1,
     107,    79,   108,    -1,   107,    83,   108,    -1,   107,    82,
     108,    -1,   107,    81,   108,    -1,   108,    66,   108,    -1,
     108,    67,   108,    -1,   108,    63,   108,    -1,   108,    64,
     108,    -1,    88,   108,    89,    -1,   109,    -1,   107,    14,
      -1,    14,   107,    -1,   107,    15,    -1,    15,   107,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   121,   121,   130,   160,   193,   198,   207,   216,   227,
     238,   247,   260,   266,   271,   295,   300,   305,   313,   323,
     325,   332,   349,   354,   360,   368,   370,   375,   379,   386,
     387,   394,   401,   408,   415,   422,   429,   436,   448,   454,
     460,   466,   472,   481,   487,   494,   509,   519,   526,   527,
     528,   589,   596,   597,   604,   611,   618,   625,   632,   639,
     646,   653,   660,   667,   668,   673,   679,   685,   691
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "IDENTIFIER", "CONSTANT", "STRING",
  "TYPE", "DYNINST_CALL", "NUMBER", "ERROR", "SIZEOF", "TRUE", "FALSE",
  "PTR_OP", "INC_OP", "DEC_OP", "LEFT_OP", "RIGHT_OP", "LEFT_ASSIGN",
  "RIGHT_ASSIGN", "AND_ASSIGN", "XOR_ASSIGN", "OR_ASSIGN", "TYPE_NAME",
  "TYPEDEF", "EXTERN", "STATIC", "CHAR", "SHORT", "INT", "LONG", "SIGNED",
  "UNSIGNED", "FLOAT", "DOUBLE", "CONST", "VOID", "STRUCT", "UNION",
  "ENUM", "ELLIPSIS", "IF", "LOCAL", "PARAM", "THREAD", "GLOBAL",
  "MACHINE", "CASE", "DEFAULT", "SWITCH", "BREAK", "RETURN", "COMMA",
  "AMPERSAND", "ASTERISK", "DOT", "NOT", "OR", "AND", "NOT_EQ", "EQ",
  "GREATER_EQ", "LESS_EQ", "'+'", "'-'", "'*'", "'/'", "'%'", "COLON",
  "SEMI", "END_BLOCK", "START_BLOCK", "NCLOSE", "NOPEN", "DOLLAR",
  "BACKTICK", "'!'", "'~'", "ASSIGN", "SUB_ASSIGN", "ADD_ASSIGN",
  "MOD_ASSIGN", "DIV_ASSIGN", "MUL_ASSIGN", "OR_OP", "AND_OP",
  "LOWER_THAN_ELSE", "ELSE", "'('", "')'", "'{'", "'}'", "'<'", "'>'",
  "$accept", "start", "var_declaration", "var_decl_modifiers",
  "statement_list", "statement", "block", "func_call", "param_list",
  "param", "bool_constant", "bool_expression", "var_modifiers",
  "variable_expr", "arith_expression", "inc_decr_expr", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,    43,    45,    42,    47,    37,   318,   319,
     320,   321,   322,   323,   324,   325,    33,   126,   326,   327,
     328,   329,   330,   331,   332,   333,   334,   335,    40,    41,
     123,   125,    60,    62
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    94,    95,    96,    96,    97,    97,    97,    97,    97,
      97,    97,    98,    98,    98,    99,    99,    99,    99,   100,
     100,   101,   102,   102,   102,   103,   103,   104,   104,   105,
     105,   105,   105,   105,   105,   105,   105,   105,   106,   106,
     106,   106,   106,   107,   107,   107,   107,   107,   108,   108,
     108,   108,   108,   108,   108,   108,   108,   108,   108,   108,
     108,   108,   108,   108,   108,   109,   109,   109,   109
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     2,     4,     1,     2,     2,     2,     2,
       2,     2,     1,     2,     3,     2,     2,     5,     7,     1,
       3,     5,     0,     1,     3,     1,     1,     1,     1,     1,
       3,     3,     3,     3,     3,     3,     3,     3,     1,     1,
       1,     1,     1,     1,     2,     3,     3,     3,     1,     1,
       1,     3,     1,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     1,     2,     2,     2,     2
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,    43,     5,    50,    49,     0,     0,     0,     0,     0,
      39,    40,    41,    38,    42,     0,     0,     0,     0,     0,
       0,     2,    12,    52,     0,    48,     0,    64,     0,    39,
      41,    38,    42,     0,    66,    68,     0,     0,     0,     0,
       6,     7,     0,     9,    10,     8,    11,     0,    44,     0,
       1,    15,     3,    13,     0,    65,    67,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    16,    47,
      44,    27,    28,    29,     0,     0,    14,    22,    63,     0,
      45,    46,    53,    55,    54,    58,    57,    56,    61,    62,
      51,    59,    60,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    26,     0,    23,    25,     4,    37,    36,     0,
      19,    17,    35,    32,    34,    33,    30,    31,     0,    21,
       0,     0,    24,    20,    18
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,    18,    19,    20,    21,    22,   111,    23,   103,   104,
      73,    74,    24,    25,    26,    27
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -84
static const yytype_int16 yypact[] =
{
     158,   -24,   -84,   -84,   -84,    27,    27,   110,   110,   -83,
     110,   -84,   110,   110,   110,   158,    33,   282,    38,   -18,
      49,   234,   -84,   -84,   -16,    52,   266,   -84,    58,   -84,
     -84,   -84,   -84,    60,   -84,   -84,   110,   110,   110,   110,
     -84,   -84,    12,   -84,   -84,   -84,   -84,   212,   -23,   122,
     -84,   -84,    -3,   -84,     4,   -84,   -84,   282,   282,   282,
     282,   282,   282,   282,   282,   282,   282,   282,   -84,   -84,
     -84,   -84,   -84,   -84,   -36,   251,   -84,   259,   -84,   282,
     -84,   -84,   -84,   -84,   -84,   -84,   -84,   -84,   -49,   -49,
     -84,   -84,   -84,    12,    12,   134,   282,   282,   282,   282,
     282,   282,   -84,   -50,   -84,    40,    40,     6,   -84,   158,
     -84,   -19,    40,    40,    40,    40,    40,    40,   259,   -84,
      83,   134,   -84,   -84,   -84
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -84,   -84,   -84,    -4,   -14,   -10,   -47,   -84,   -84,   -40,
     -84,   -80,   -84,    23,   -17,   -84
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint8 yytable[] =
{
      49,    47,   118,    40,    41,    42,    43,    80,    44,    45,
      46,    53,    81,   107,   108,     1,    65,    66,    67,     3,
       4,    93,    94,    71,    72,    75,     5,     6,    34,    35,
       1,    28,    43,    44,    45,    46,    48,    53,    50,   119,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    51,    52,    95,    29,    11,    30,    31,    32,    54,
     105,    69,   106,    70,    94,    77,    55,    56,   121,    29,
      11,    30,    31,    32,   124,    79,    75,    75,   122,   112,
     113,   114,   115,   116,   117,   110,     1,    16,     0,     2,
       3,     4,     0,     0,     0,   120,     0,     5,     6,     0,
      17,   105,    33,    63,    64,    65,    66,    67,     0,     7,
      53,   110,     0,     0,     0,     0,     2,     0,     8,     0,
       0,     0,     0,     0,     9,    10,    11,    12,    13,    14,
      57,    58,    59,    60,    61,    62,     7,     1,     0,     0,
       2,     3,     4,     0,     0,     8,     0,     0,     5,     6,
       0,     0,    36,     0,    37,    38,    39,     0,    16,     0,
       7,     1,     0,     0,     2,     3,     4,     0,     0,     8,
       0,    17,     5,     6,   123,     9,    10,    11,    12,    13,
      14,     0,     0,     0,     7,    63,    64,    65,    66,    67,
       0,     0,     0,     8,     0,     0,     0,     0,     0,     9,
      10,    11,    12,    13,    14,     0,     0,     0,     0,    16,
       0,    78,     0,     0,     0,     1,     0,     0,     2,     3,
       4,     0,    17,     0,   109,     0,     5,     6,     0,     0,
       0,    15,     0,    16,     0,     0,     0,     1,     7,     0,
       2,     3,     4,     0,     0,     0,    17,     8,     5,     6,
       0,     0,     0,     9,    10,    11,    12,    13,    14,     0,
       7,     0,     1,     0,   102,     0,     3,     4,     0,     8,
       0,     0,     0,     5,     6,     9,    10,    11,    12,    13,
      14,     0,     0,     0,    76,     1,     0,    16,     0,     3,
       4,     0,     0,     0,     0,     0,     5,     6,     0,     0,
      17,    29,    11,    30,    31,    32,     0,     0,     0,    16,
      96,    97,    98,    99,    63,    64,    65,    66,    67,     0,
       0,     0,    17,     0,    29,    11,    30,    31,    32,    63,
      64,    65,    66,    67,    16,    68,     0,     0,     0,     0,
       0,     0,     0,   100,   101,     0,     0,    17,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    16,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      17
};

static const yytype_int8 yycheck[] =
{
      17,    15,    52,     7,     8,    88,    10,     3,    12,    13,
      14,    21,     8,    93,    94,     3,    65,    66,    67,     7,
       8,    57,    58,    11,    12,    42,    14,    15,     5,     6,
       3,    55,    36,    37,    38,    39,     3,    47,     0,    89,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    69,     3,    89,    42,    43,    44,    45,    46,    75,
      77,     3,    79,     3,    58,    88,    14,    15,    87,    42,
      43,    44,    45,    46,   121,    78,    93,    94,   118,    96,
      97,    98,    99,   100,   101,    95,     3,    75,    -1,     6,
       7,     8,    -1,    -1,    -1,   109,    -1,    14,    15,    -1,
      88,   118,    75,    63,    64,    65,    66,    67,    -1,    26,
     120,   121,    -1,    -1,    -1,    -1,     6,    -1,    35,    -1,
      -1,    -1,    -1,    -1,    41,    42,    43,    44,    45,    46,
      78,    79,    80,    81,    82,    83,    26,     3,    -1,    -1,
       6,     7,     8,    -1,    -1,    35,    -1,    -1,    14,    15,
      -1,    -1,    42,    -1,    44,    45,    46,    -1,    75,    -1,
      26,     3,    -1,    -1,     6,     7,     8,    -1,    -1,    35,
      -1,    88,    14,    15,    91,    41,    42,    43,    44,    45,
      46,    -1,    -1,    -1,    26,    63,    64,    65,    66,    67,
      -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,    -1,    41,
      42,    43,    44,    45,    46,    -1,    -1,    -1,    -1,    75,
      -1,    89,    -1,    -1,    -1,     3,    -1,    -1,     6,     7,
       8,    -1,    88,    -1,    90,    -1,    14,    15,    -1,    -1,
      -1,    73,    -1,    75,    -1,    -1,    -1,     3,    26,    -1,
       6,     7,     8,    -1,    -1,    -1,    88,    35,    14,    15,
      -1,    -1,    -1,    41,    42,    43,    44,    45,    46,    -1,
      26,    -1,     3,    -1,     5,    -1,     7,     8,    -1,    35,
      -1,    -1,    -1,    14,    15,    41,    42,    43,    44,    45,
      46,    -1,    -1,    -1,    72,     3,    -1,    75,    -1,     7,
       8,    -1,    -1,    -1,    -1,    -1,    14,    15,    -1,    -1,
      88,    42,    43,    44,    45,    46,    -1,    -1,    -1,    75,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    -1,
      -1,    -1,    88,    -1,    42,    43,    44,    45,    46,    63,
      64,    65,    66,    67,    75,    69,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    92,    93,    -1,    -1,    88,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    75,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      88
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,     6,     7,     8,    14,    15,    26,    35,    41,
      42,    43,    44,    45,    46,    73,    75,    88,    95,    96,
      97,    98,    99,   101,   106,   107,   108,   109,    55,    42,
      44,    45,    46,    75,   107,   107,    42,    44,    45,    46,
      97,    97,    88,    97,    97,    97,    97,    98,     3,   108,
       0,    69,     3,    99,    75,    14,    15,    78,    79,    80,
      81,    82,    83,    63,    64,    65,    66,    67,    69,     3,
       3,    11,    12,   104,   105,   108,    72,    88,    89,    78,
       3,     8,   108,   108,   108,   108,   108,   108,   108,   108,
     108,   108,   108,    57,    58,    89,    59,    60,    61,    62,
      92,    93,     5,   102,   103,   108,   108,   105,   105,    90,
      99,   100,   108,   108,   108,   108,   108,   108,    52,    89,
      98,    87,   103,    91,   100
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *bottom, yytype_int16 *top)
#else
static void
yy_stack_print (bottom, top)
    yytype_int16 *bottom;
    yytype_int16 *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      fprintf (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      fprintf (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The look-ahead symbol.  */
int yychar;

/* The semantic value of the look-ahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
  int yystate;
  int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  int yytoken = 0;
#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  yytype_int16 yyssa[YYINITDEPTH];
  yytype_int16 *yyss = yyssa;
  yytype_int16 *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  YYSTYPE *yyvsp;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     look-ahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to look-ahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a look-ahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid look-ahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 122 "../src/C.y"
    { 
       (yyvsp[(1) - (1)].snippetList)->insert((yyvsp[(1) - (1)].snippetList)->end(), endSnippets.begin(), endSnippets.end());
       parse_result = new BPatch_sequence(*(yyvsp[(1) - (1)].snippetList)); 
       delete (yyvsp[(1) - (1)].snippetList);
       endSnippets.clear();
    ;}
    break;

  case 3:
#line 131 "../src/C.y"
    {
       //IDENTIFIER leaks, but how to fix b/c use of $0?
       std::vector<BPatch_snippet *> argVect;
       std::string declString;
       std::string mangledName = dynC_API::mangle((yyvsp[(2) - (2)].sval), (yyvsp[(1) - (2)].varSpec).type, (yyvsp[(1) - (2)].varSpec).isGlobal);
       if(verbose)printf("mangled = %s\n", mangledName.c_str());
       if(verbose)printf("type = %s\n", (yyvsp[(1) - (2)].varSpec).type);
       argVect.push_back(new BPatch_constExpr(strdup(mangledName.c_str()))); // string (name)
       argVect.push_back(new BPatch_constExpr(strdup((yyvsp[(1) - (2)].varSpec).type)));// string (type)
       
       if((yyvsp[(1) - (2)].varSpec).isStatic) declString += "static ";
       if((yyvsp[(1) - (2)].varSpec).isConstant) declString += "const ";

       if((yyvsp[(1) - (2)].varSpec).isLocal) declString += "local ";
       if((yyvsp[(1) - (2)].varSpec).isThread) declString += "thread ";
       
       //machine state?
       
//       argVect.push_back(new BPatch_constExpr(declString.c_str()));
       //have global list of snippets to put at the end of the ast.
       (yyval.snippet) = new BPatch_utilExpr(BPatch_DyninstAllocate, argVect); //will only allocate memory if nessessary
       if(!((yyvsp[(1) - (2)].varSpec).isStatic || (yyvsp[(1) - (2)].varSpec).isGlobal)){ 
          BPatch_snippet setSn = BPatch_constExpr(0); // if int etc.
          if(strcmp((yyvsp[(1) - (2)].varSpec).type, "char") == 0){
             setSn = BPatch_constExpr("");
          }
          endSnippets.push_back(new BPatch_arithExpr(BPatch_assign, *(yyval.snippet), setSn));
       }
    ;}
    break;

  case 4:
#line 161 "../src/C.y"
    {   
       std::vector<BPatch_snippet *> argVect;
       std::string declString;
       std::string mangledName = dynC_API::mangle((yyvsp[(2) - (4)].sval), (yyvsp[(1) - (4)].varSpec).type, (yyvsp[(1) - (4)].varSpec).isGlobal);
       if(verbose)printf("mangled = %s\n", mangledName.c_str());
       if(verbose)printf("type = %s\n", (yyvsp[(1) - (4)].varSpec).type);
       argVect.push_back(new BPatch_constExpr(strdup(mangledName.c_str()))); // string (name)
       argVect.push_back(new BPatch_constExpr(strdup((yyvsp[(1) - (4)].varSpec).type)));// string (type)


       if((yyvsp[(1) - (4)].varSpec).isStatic) declString += "static ";
       if((yyvsp[(1) - (4)].varSpec).isConstant) declString += "const ";
       
       if((yyvsp[(1) - (4)].varSpec).isLocal) declString += "local ";
       if((yyvsp[(1) - (4)].varSpec).isThread) declString += "thread ";
       
       //machine state?
       
//       argVect.push_back(new BPatch_constExpr(declString.c_str()));
       //have global list of snippets to put at the end of the ast.
       if((yyvsp[(1) - (4)].varSpec).isStatic || (yyvsp[(1) - (4)].varSpec).isGlobal){
          argVect.push_back((yyvsp[(4) - (4)].snippet));
          (yyval.snippet) = new BPatch_utilExpr(BPatch_DyninstAllocateAndAssign, argVect); //will only allocate memory if nessessary
       }else{
          BPatch_utilExpr alloc = BPatch_utilExpr(BPatch_DyninstAllocate, argVect);
          std::vector<BPatch_snippet *> varArgs;
          varArgs.push_back(new BPatch_constExpr(mangledName.c_str()));
          BPatch_arithExpr assign = BPatch_arithExpr(BPatch_assign, BPatch_utilExpr(BPatch_InstVar, varArgs), *(yyvsp[(4) - (4)].snippet));
          (yyval.snippet) = new BPatch_arithExpr(BPatch_seq, alloc, assign); //will only allocate memory if nessessary
       }
    ;}
    break;

  case 5:
#line 194 "../src/C.y"
    {
      YYSTYPE::VariableSpec rSpec = {false,false,false,false,false,false,false,false,(yyvsp[(1) - (1)].sval)};
      (yyval.varSpec) = rSpec;
   ;}
    break;

  case 6:
#line 199 "../src/C.y"
    {
      if ((yyvsp[(2) - (2)].varSpec).isStatic){
         //throw error: two statics
      }else{
         (yyvsp[(2) - (2)].varSpec).isStatic = true;
      }
      (yyval.varSpec) = (yyvsp[(2) - (2)].varSpec);
   ;}
    break;

  case 7:
#line 208 "../src/C.y"
    {
      if ((yyvsp[(2) - (2)].varSpec).isConstant){
         //throw error: two consts
      }else{
         (yyvsp[(2) - (2)].varSpec).isConstant = true;
      }
      (yyval.varSpec) = (yyvsp[(2) - (2)].varSpec);
   ;}
    break;

  case 8:
#line 217 "../src/C.y"
    {
      if ((yyvsp[(2) - (2)].varSpec).isGlobal){
         //throw error: two consts
      }else if((yyvsp[(2) - (2)].varSpec).isLocal){
         //throw error: can't be global and local
      }else {
         (yyvsp[(2) - (2)].varSpec).isGlobal = true;
      }
      (yyval.varSpec) = (yyvsp[(2) - (2)].varSpec);   
   ;}
    break;

  case 9:
#line 228 "../src/C.y"
    {
      if ((yyvsp[(2) - (2)].varSpec).isLocal){
         //throw error: two consts
      }else if((yyvsp[(2) - (2)].varSpec).isGlobal){
         //throw error: can't be global and local
      }else{
         (yyvsp[(2) - (2)].varSpec).isLocal = true;
      }
      (yyval.varSpec) = (yyvsp[(2) - (2)].varSpec);   
   ;}
    break;

  case 10:
#line 239 "../src/C.y"
    {
      if ((yyvsp[(2) - (2)].varSpec).isThread){
         //throw error: two consts
      }else{
         (yyvsp[(2) - (2)].varSpec).isThread = true;
      }
      (yyval.varSpec) = (yyvsp[(2) - (2)].varSpec);   
   ;}
    break;

  case 11:
#line 248 "../src/C.y"
    {
      if ((yyvsp[(2) - (2)].varSpec).isMachineState){
         //throw error: two consts
      }else{
         (yyvsp[(2) - (2)].varSpec).isMachineState = true;
      }
      (yyval.varSpec) = (yyvsp[(2) - (2)].varSpec);   
   ;}
    break;

  case 12:
#line 261 "../src/C.y"
    { 
       if(verbose) printf("\n");
       (yyval.snippetList) = new BPatch_Vector<BPatch_snippet *>; 
       (yyval.snippetList)->push_back((yyvsp[(1) - (1)].snippet));
    ;}
    break;

  case 13:
#line 267 "../src/C.y"
    {
       (yyvsp[(1) - (2)].snippetList)->push_back((yyvsp[(2) - (2)].snippet));
       (yyval.snippetList) = (yyvsp[(1) - (2)].snippetList);
    ;}
    break;

  case 14:
#line 272 "../src/C.y"
    {
       
       //one time code
       std::vector<BPatch_snippet *> *retVect = new std::vector<BPatch_snippet *>;
       std::vector<BPatch_snippet *> argVect;
       
       argVect.push_back(new BPatch_constExpr("int")); // type
       BPatch_utilExpr *onetime = new BPatch_utilExpr(BPatch_DyninstAllocate, argVect); 
       
       retVect->push_back(onetime);
       BPatch_arithExpr *addOne = new BPatch_arithExpr(BPatch_assign, *onetime, BPatch_constExpr(1));
       (yyvsp[(2) - (3)].snippetList)->push_back(addOne);
       BPatch_sequence *seq = new BPatch_sequence(*(yyvsp[(2) - (3)].snippetList));

       BPatch_boolExpr *isZero = new BPatch_boolExpr(BPatch_eq, *onetime, BPatch_constExpr(0));

       retVect->push_back(new BPatch_ifExpr(*isZero, *seq));
       (yyval.snippetList) = retVect;       
           
       ;}
    break;

  case 15:
#line 296 "../src/C.y"
    {
       (yyval.snippet) = (yyvsp[(1) - (2)].snippet);
    ;}
    break;

  case 16:
#line 301 "../src/C.y"
    { 
       (yyval.snippet) = (yyvsp[(1) - (2)].snippet); 
    ;}
    break;

  case 17:
#line 306 "../src/C.y"
    {
       if(verbose) printf(" if () ");
       (yyval.snippet) = new BPatch_ifExpr(*(yyvsp[(3) - (5)].boolExpr), *(yyvsp[(5) - (5)].snippet));
       delete (yyvsp[(3) - (5)].boolExpr);
       delete (yyvsp[(5) - (5)].snippet);
    ;}
    break;

  case 18:
#line 314 "../src/C.y"
    {
       if(verbose) printf(" if () else ");
       (yyval.snippet) = new BPatch_ifExpr(*(yyvsp[(3) - (7)].boolExpr), *(yyvsp[(5) - (7)].snippet), *(yyvsp[(7) - (7)].snippet));
       delete (yyvsp[(3) - (7)].boolExpr);
       delete (yyvsp[(5) - (7)].snippet);
       delete (yyvsp[(7) - (7)].snippet);
    ;}
    break;

  case 20:
#line 326 "../src/C.y"
    {
        (yyval.snippet) = new BPatch_sequence(*(yyvsp[(2) - (3)].snippetList));
        delete (yyvsp[(2) - (3)].snippetList);
     ;}
    break;

  case 21:
#line 334 "../src/C.y"
    { 
       
       if(verbose) printf(" %s () ", (yyvsp[(2) - (5)].sval));
       std::vector<BPatch_snippet *> argVect;
       argVect.push_back(new BPatch_constExpr((yyvsp[(2) - (5)].sval)));
       argVect.insert(argVect.end(), (*(yyvsp[(4) - (5)].snippetList)).begin(), (*(yyvsp[(4) - (5)].snippetList)).end());
//       free($2);
       (yyval.snippet) = new BPatch_utilExpr(BPatch_AppFunction, argVect);
       delete (yyvsp[(4) - (5)].snippetList);
       
    ;}
    break;

  case 22:
#line 349 "../src/C.y"
    {
       //No parameters, return an empty vector
       (yyval.snippetList) = new BPatch_Vector<BPatch_snippet *>;
    ;}
    break;

  case 23:
#line 355 "../src/C.y"
    { 
       (yyval.snippetList) = new BPatch_Vector<BPatch_snippet *>; 
       (yyval.snippetList)->push_back((yyvsp[(1) - (1)].snippet));
    ;}
    break;

  case 24:
#line 361 "../src/C.y"
    { 
       if(verbose) printf(" , ");
       (yyvsp[(1) - (3)].snippetList)->push_back((yyvsp[(3) - (3)].snippet)); 
       (yyval.snippetList) = (yyvsp[(1) - (3)].snippetList);
    ;}
    break;

  case 26:
#line 370 "../src/C.y"
    { 
       (yyval.snippet) = new BPatch_constExpr((yyvsp[(1) - (1)].sval)); 
    ;}
    break;

  case 27:
#line 376 "../src/C.y"
    { 
       (yyval.boolExpr) = new BPatch_boolExpr(BPatch_eq, BPatch_constExpr(0), BPatch_constExpr(0));
    ;}
    break;

  case 28:
#line 380 "../src/C.y"
    {
       (yyval.boolExpr) = new BPatch_boolExpr(BPatch_ne, BPatch_constExpr(0), BPatch_constExpr(0));
    ;}
    break;

  case 30:
#line 388 "../src/C.y"
    {
       if(verbose) printf(" < ");
       (yyval.boolExpr) = new BPatch_boolExpr(BPatch_lt, *(yyvsp[(1) - (3)].snippet), *(yyvsp[(3) - (3)].snippet));
       delete (yyvsp[(1) - (3)].snippet);
       delete (yyvsp[(3) - (3)].snippet);
    ;}
    break;

  case 31:
#line 395 "../src/C.y"
    {
       if(verbose) printf(" > ");
       (yyval.boolExpr) = new BPatch_boolExpr(BPatch_gt, *(yyvsp[(1) - (3)].snippet), *(yyvsp[(3) - (3)].snippet));
       delete (yyvsp[(1) - (3)].snippet);
       delete (yyvsp[(3) - (3)].snippet);
    ;}
    break;

  case 32:
#line 402 "../src/C.y"
    {
       if(verbose) printf(" == ");
       (yyval.boolExpr) = new BPatch_boolExpr(BPatch_eq, *(yyvsp[(1) - (3)].snippet), *(yyvsp[(3) - (3)].snippet));
       delete (yyvsp[(1) - (3)].snippet);
       delete (yyvsp[(3) - (3)].snippet);
    ;}
    break;

  case 33:
#line 409 "../src/C.y"
    {
       if(verbose) printf(" <= ");
       (yyval.boolExpr) = new BPatch_boolExpr(BPatch_le, *(yyvsp[(1) - (3)].snippet), *(yyvsp[(3) - (3)].snippet));
       delete (yyvsp[(1) - (3)].snippet);
       delete (yyvsp[(3) - (3)].snippet);
    ;}
    break;

  case 34:
#line 416 "../src/C.y"
    {
       if(verbose) printf(" >= ");
       (yyval.boolExpr) = new BPatch_boolExpr(BPatch_ge, *(yyvsp[(1) - (3)].snippet), *(yyvsp[(3) - (3)].snippet));
       delete (yyvsp[(1) - (3)].snippet);
       delete (yyvsp[(3) - (3)].snippet);
    ;}
    break;

  case 35:
#line 423 "../src/C.y"
    {
       if(verbose) printf(" != ");
       (yyval.boolExpr) = new BPatch_boolExpr(BPatch_ne, *(yyvsp[(1) - (3)].snippet), *(yyvsp[(3) - (3)].snippet));
       delete (yyvsp[(1) - (3)].snippet);
       delete (yyvsp[(3) - (3)].snippet);
    ;}
    break;

  case 36:
#line 430 "../src/C.y"
    {
       if(verbose) printf(" AND ");
       (yyval.boolExpr) = new BPatch_boolExpr(BPatch_and, *(yyvsp[(1) - (3)].boolExpr), *(yyvsp[(3) - (3)].boolExpr));
       delete (yyvsp[(1) - (3)].boolExpr);
       delete (yyvsp[(3) - (3)].boolExpr);
    ;}
    break;

  case 37:
#line 437 "../src/C.y"
    {       if(verbose) printf(" OR ");
       (yyval.boolExpr) = new BPatch_boolExpr(BPatch_or, *(yyvsp[(1) - (3)].boolExpr), *(yyvsp[(3) - (3)].boolExpr));
       delete (yyvsp[(1) - (3)].boolExpr);
       delete (yyvsp[(3) - (3)].boolExpr);
    ;}
    break;

  case 38:
#line 449 "../src/C.y"
    {
      YYSTYPE::VariableSpec vSpec = {false,false,false,false,false,false,false,false,""};
      vSpec.isGlobal = true;
      (yyval.varSpec) = vSpec;   
   ;}
    break;

  case 39:
#line 455 "../src/C.y"
    {
      YYSTYPE::VariableSpec vSpec = {false,false,false,false,false,false,false,false,""};
      vSpec.isLocal = true;
      (yyval.varSpec) = vSpec;   
   ;}
    break;

  case 40:
#line 461 "../src/C.y"
    {
      YYSTYPE::VariableSpec vSpec = {false,false,false,false,false,false,false,false,""};
      vSpec.isParam = true;
      (yyval.varSpec) = vSpec;   
   ;}
    break;

  case 41:
#line 467 "../src/C.y"
    {
     YYSTYPE::VariableSpec vSpec = {false,false,false,false,false,false,false,false,""};
      vSpec.isThread = true;
      (yyval.varSpec) = vSpec;   
   ;}
    break;

  case 42:
#line 473 "../src/C.y"
    {
      YYSTYPE::VariableSpec vSpec = {false,false,false,false,false,false,false,false,""};
      vSpec.isMachineState = true;
      (yyval.varSpec) = vSpec;   
   ;}
    break;

  case 43:
#line 482 "../src/C.y"
    {
       std::vector<BPatch_snippet *> argVect;
       argVect.push_back(new BPatch_constExpr(dynC_API::getMangledStub((yyvsp[(1) - (1)].sval)).c_str()));
       (yyval.snippet) = new BPatch_utilExpr(BPatch_InstVar, argVect);
    ;}
    break;

  case 44:
#line 488 "../src/C.y"
    {
       std::vector<BPatch_snippet *> argVect;
       argVect.push_back(new BPatch_constExpr((yyvsp[(2) - (2)].sval)));
       (yyval.snippet) = new BPatch_utilExpr(BPatch_AppVar, argVect);
       if(verbose) printf("BPatch_utilExpr(BPatch_AppVar)\n");
    ;}
    break;

  case 45:
#line 495 "../src/C.y"
    {
        std::vector<BPatch_snippet *> argVect;
        argVect.push_back(new BPatch_constExpr((yyvsp[(3) - (3)].sval)));
        
        BPatch_utilKind uk = BPatch_AppVar;
        
        if((yyvsp[(1) - (3)].varSpec).isGlobal) uk = BPatch_AppGlobalVar;
        if((yyvsp[(1) - (3)].varSpec).isLocal) uk = BPatch_AppLocalVar;
        if((yyvsp[(1) - (3)].varSpec).isParam) uk = BPatch_AppParamVar;
        if((yyvsp[(1) - (3)].varSpec).isThread) uk = BPatch_AppThreadVar;
        if((yyvsp[(1) - (3)].varSpec).isMachineState) uk = BPatch_AppMachineState;

        (yyval.snippet) = new BPatch_utilExpr(uk, argVect);
    ;}
    break;

  case 46:
#line 510 "../src/C.y"
    {
       //special case for indexed parameters
       if(!(yyvsp[(1) - (3)].varSpec).isParam){
          yyerror("Woops!");
       }
       std::vector<BPatch_snippet *> argVect;
       argVect.push_back(new BPatch_constExpr((yyvsp[(3) - (3)].ival)));
       (yyval.snippet) = new BPatch_utilExpr(BPatch_AppParamVar, argVect);
    ;}
    break;

  case 47:
#line 520 "../src/C.y"
    {
       //impliment compound types later.
    ;}
    break;

  case 49:
#line 527 "../src/C.y"
    { if(verbose) printf(" %d ", (yyvsp[(1) - (1)].ival)); (yyval.snippet) = new BPatch_constExpr((yyvsp[(1) - (1)].ival));;}
    break;

  case 50:
#line 529 "../src/C.y"
    {
       if(verbose) printf(" _$%s ", (yyvsp[(1) - (1)].sval));
       
       std::vector<BPatch_snippet *> argVect;
       //snippets w/ return vals
       if(strcmp((yyvsp[(1) - (1)].sval), "function_name") == 0){
          (yyval.snippet) = new BPatch_utilExpr(BPatch_DyninstFunctionName, argVect);
          break;
       }
       if(strcmp((yyvsp[(1) - (1)].sval), "point") == 0){
          (yyval.snippet) = new BPatch_utilExpr(BPatch_DyninstPointDescription, argVect); 
           break;
       }
       if(strcmp((yyvsp[(1) - (1)].sval), "module_name") == 0){
          (yyval.snippet) = new BPatch_utilExpr(BPatch_DyninstModuleName, argVect); 
           break;
       }
       if(strcmp((yyvsp[(1) - (1)].sval), "bytes_accessed") == 0){
          (yyval.snippet) = new BPatch_bytesAccessedExpr();
          break;
       }
       if(strcmp((yyvsp[(1) - (1)].sval), "effective_address") == 0){
          (yyval.snippet) = new BPatch_effectiveAddressExpr();
          break;
       }
       if(strcmp((yyvsp[(1) - (1)].sval), "original_address") == 0){
          (yyval.snippet) = new BPatch_originalAddressExpr();
          break;
       }
       if(strcmp((yyvsp[(1) - (1)].sval), "actual_address") == 0){
          (yyval.snippet) = new BPatch_actualAddressExpr();
          break;
       }
       if(strcmp((yyvsp[(1) - (1)].sval), "return_value") == 0){
          (yyval.snippet) = new BPatch_retExpr();
          break;
       }
       if(strcmp((yyvsp[(1) - (1)].sval), "thread_index") == 0){
          (yyval.snippet) = new BPatch_threadIndexExpr();
          break;
       }
       if(strcmp((yyvsp[(1) - (1)].sval), "tid") == 0){
          (yyval.snippet) = new BPatch_utilExpr(BPatch_DyninstTID, argVect); 
          break;
       }

       //snippets w/out return vals. Need type checking!
       
       if(strcmp((yyvsp[(1) - (1)].sval), "break") == 0){
          (yyval.snippet) = new BPatch_breakPointExpr();
          break;
       }
       
       if(strcmp((yyvsp[(1) - (1)].sval), "stop_thread") == 0){
          //how?
          break;
       }
       //TODO: improve error reporting to include name 
       yyerror("Syntax error: unrecognized dyninst call");
    ;}
    break;

  case 51:
#line 590 "../src/C.y"
    {
       if(verbose) printf(" * ");
       (yyval.snippet) = new BPatch_arithExpr(BPatch_times, *(yyvsp[(1) - (3)].snippet), *(yyvsp[(3) - (3)].snippet));
       delete (yyvsp[(1) - (3)].snippet);
       delete (yyvsp[(3) - (3)].snippet);
    ;}
    break;

  case 52:
#line 596 "../src/C.y"
    {(yyval.snippet) = (yyvsp[(1) - (1)].snippet);;}
    break;

  case 53:
#line 598 "../src/C.y"
    {
       if(verbose) printf(" = ");
	    (yyval.snippet) = new BPatch_arithExpr(BPatch_assign, *(yyvsp[(1) - (3)].snippet), *(yyvsp[(3) - (3)].snippet));
	    delete (yyvsp[(1) - (3)].snippet);
	    delete (yyvsp[(3) - (3)].snippet);
    ;}
    break;

  case 54:
#line 605 "../src/C.y"
    {
       if(verbose) printf(" += ");
       (yyval.snippet) = new BPatch_arithExpr(BPatch_assign, *(yyvsp[(1) - (3)].snippet), BPatch_arithExpr(BPatch_plus, *(yyvsp[(1) - (3)].snippet), *(yyvsp[(3) - (3)].snippet)));
       delete (yyvsp[(1) - (3)].snippet);
       delete (yyvsp[(3) - (3)].snippet);
    ;}
    break;

  case 55:
#line 612 "../src/C.y"
    {
       if(verbose) printf(" -= ");
       (yyval.snippet) = new BPatch_arithExpr(BPatch_assign, *(yyvsp[(1) - (3)].snippet), BPatch_arithExpr(BPatch_minus, *(yyvsp[(1) - (3)].snippet), *(yyvsp[(3) - (3)].snippet)));
       delete (yyvsp[(1) - (3)].snippet);
       delete (yyvsp[(3) - (3)].snippet);
    ;}
    break;

  case 56:
#line 619 "../src/C.y"
    {
       if(verbose) printf(" *= ");
       (yyval.snippet) = new BPatch_arithExpr(BPatch_assign, *(yyvsp[(1) - (3)].snippet), BPatch_arithExpr(BPatch_times, *(yyvsp[(1) - (3)].snippet), *(yyvsp[(3) - (3)].snippet)));
       delete (yyvsp[(1) - (3)].snippet);
       delete (yyvsp[(3) - (3)].snippet);
    ;}
    break;

  case 57:
#line 626 "../src/C.y"
    {
       if(verbose) printf(" /= ");
       (yyval.snippet) = new BPatch_arithExpr(BPatch_assign, *(yyvsp[(1) - (3)].snippet), BPatch_arithExpr(BPatch_divide, *(yyvsp[(1) - (3)].snippet), *(yyvsp[(3) - (3)].snippet)));
       delete (yyvsp[(1) - (3)].snippet);
       delete (yyvsp[(3) - (3)].snippet);
    ;}
    break;

  case 58:
#line 633 "../src/C.y"
    {
       if(verbose) printf(" %%= ");
       (yyval.snippet) = new BPatch_arithExpr(BPatch_assign, *(yyvsp[(1) - (3)].snippet), BPatch_arithExpr(BPatch_minus, *(yyvsp[(1) - (3)].snippet), BPatch_arithExpr(BPatch_times, BPatch_arithExpr(BPatch_divide, *(yyvsp[(1) - (3)].snippet), *(yyvsp[(3) - (3)].snippet)), *(yyvsp[(3) - (3)].snippet))));
       delete (yyvsp[(1) - (3)].snippet);
       delete (yyvsp[(3) - (3)].snippet);
    ;}
    break;

  case 59:
#line 640 "../src/C.y"
    {
       if(verbose) printf(" / ");
       (yyval.snippet) = new BPatch_arithExpr(BPatch_divide, *(yyvsp[(1) - (3)].snippet), *(yyvsp[(3) - (3)].snippet));
       delete (yyvsp[(1) - (3)].snippet);
       delete (yyvsp[(3) - (3)].snippet);
    ;}
    break;

  case 60:
#line 647 "../src/C.y"
    {
       if(verbose) printf(" %% ");
       (yyval.snippet) = new BPatch_arithExpr(BPatch_minus, *(yyvsp[(1) - (3)].snippet), BPatch_arithExpr(BPatch_times, BPatch_arithExpr(BPatch_divide, *(yyvsp[(1) - (3)].snippet), *(yyvsp[(3) - (3)].snippet)), *(yyvsp[(3) - (3)].snippet)));
       delete (yyvsp[(1) - (3)].snippet);
       delete (yyvsp[(3) - (3)].snippet);
    ;}
    break;

  case 61:
#line 654 "../src/C.y"
    {
       if(verbose) printf(" + ");
       (yyval.snippet) = new BPatch_arithExpr(BPatch_plus, *(yyvsp[(1) - (3)].snippet), *(yyvsp[(3) - (3)].snippet));
       delete (yyvsp[(1) - (3)].snippet);
       delete (yyvsp[(3) - (3)].snippet);
    ;}
    break;

  case 62:
#line 661 "../src/C.y"
    {
       if(verbose) printf(" - ");
       (yyval.snippet) = new BPatch_arithExpr(BPatch_minus, *(yyvsp[(1) - (3)].snippet), *(yyvsp[(3) - (3)].snippet));
       delete (yyvsp[(1) - (3)].snippet);
       delete (yyvsp[(3) - (3)].snippet);
    ;}
    break;

  case 63:
#line 667 "../src/C.y"
    {(yyval.snippet) = (yyvsp[(2) - (3)].snippet);;}
    break;

  case 64:
#line 668 "../src/C.y"
    {(yyval.snippet) = (yyvsp[(1) - (1)].snippet);;}
    break;

  case 65:
#line 674 "../src/C.y"
    {
       if(verbose) printf(" ++ ");
       BPatch_arithExpr addOne = BPatch_arithExpr(BPatch_assign, *(yyvsp[(1) - (2)].snippet), BPatch_arithExpr(BPatch_plus, *(yyvsp[(1) - (2)].snippet), BPatch_constExpr(1)));
       (yyval.snippet) = new BPatch_arithExpr(BPatch_seq, (BPatch_snippet &)addOne, BPatch_arithExpr(BPatch_minus, *(yyvsp[(1) - (2)].snippet), BPatch_constExpr(1)));
    ;}
    break;

  case 66:
#line 680 "../src/C.y"
    {
       if(verbose) printf(" ++ ");
       BPatch_arithExpr addOne = BPatch_arithExpr(BPatch_assign, *(yyvsp[(2) - (2)].snippet), BPatch_arithExpr(BPatch_plus, *(yyvsp[(2) - (2)].snippet), BPatch_constExpr(1)));
       (yyval.snippet) = new BPatch_arithExpr(BPatch_seq, (BPatch_snippet &)addOne, *(yyvsp[(2) - (2)].snippet));
    ;}
    break;

  case 67:
#line 686 "../src/C.y"
    {
       if(verbose) printf(" -- ");
       BPatch_arithExpr subOne = BPatch_arithExpr(BPatch_assign, *(yyvsp[(1) - (2)].snippet), BPatch_arithExpr(BPatch_minus, *(yyvsp[(1) - (2)].snippet), BPatch_constExpr(1)));
       (yyval.snippet) = new BPatch_arithExpr(BPatch_seq, (BPatch_snippet &)subOne, BPatch_arithExpr(BPatch_plus, *(yyvsp[(1) - (2)].snippet), BPatch_constExpr(1)));
    ;}
    break;

  case 68:
#line 692 "../src/C.y"
    {
       if(verbose) printf(" -- ");
       BPatch_arithExpr subOne = BPatch_arithExpr(BPatch_assign, *(yyvsp[(2) - (2)].snippet), BPatch_arithExpr(BPatch_minus, *(yyvsp[(2) - (2)].snippet), BPatch_constExpr(1)));
       (yyval.snippet) = new BPatch_arithExpr(BPatch_seq, (BPatch_snippet &)subOne, *(yyvsp[(2) - (2)].snippet));
    ;}
    break;


/* Line 1267 of yacc.c.  */
#line 2435 "dynC.tab.c"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse look-ahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse look-ahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEOF && yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}


#line 699 "../src/C.y"


#include <stdio.h>

void yyerror(char *s)
{
   fflush(stdout);
   printf("ERROR on line %d: %s\n", line_num, s);
}

void warn(char *s)
{
   fflush(stdout);
   printf("WARNING on line %d: %s\n", line_num, s);
}

void writeMessage(char *s){
   fflush(mfile);
   fprintf(mfile, "At line %d: ", line_num);
   fprintf(mfile, "%s\n", s);
   
}
 

