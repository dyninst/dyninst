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
#define YYLSP_NEEDED 1

/* Substitute the variable and function names.  */
#define yyparse dynCparse
#define yylex   dynClex
#define yyerror dynCerror
#define yylval  dynClval
#define yychar  dynCchar
#define yydebug dynCdebug
#define yynerrs dynCnerrs
#define yylloc dynClloc

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
     NEWLINE = 304,
     CASE = 305,
     DEFAULT = 306,
     SWITCH = 307,
     RETURN = 308,
     NILL = 309,
     COMMA = 310,
     AMPERSAND = 311,
     ASTERISK = 312,
     DOT = 313,
     NOT = 314,
     OR = 315,
     AND = 316,
     NOT_EQ = 317,
     EQ = 318,
     GREATER_EQ = 319,
     LESS_EQ = 320,
     COLON = 321,
     SEMI = 322,
     END_BLOCK = 323,
     START_BLOCK = 324,
     DOLLAR = 325,
     BACKTICK = 326,
     ASSIGN = 327,
     SUB_ASSIGN = 328,
     ADD_ASSIGN = 329,
     MOD_ASSIGN = 330,
     DIV_ASSIGN = 331,
     MUL_ASSIGN = 332,
     OR_OP = 333,
     AND_OP = 334,
     NCLOSE = 335,
     NOPEN = 336,
     LOWER_THAN_ELSE = 337,
     ELSE = 338,
     LOWER_THAN_DEREF = 339
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
#define NEWLINE 304
#define CASE 305
#define DEFAULT 306
#define SWITCH 307
#define RETURN 308
#define NILL 309
#define COMMA 310
#define AMPERSAND 311
#define ASTERISK 312
#define DOT 313
#define NOT 314
#define OR 315
#define AND 316
#define NOT_EQ 317
#define EQ 318
#define GREATER_EQ 319
#define LESS_EQ 320
#define COLON 321
#define SEMI 322
#define END_BLOCK 323
#define START_BLOCK 324
#define DOLLAR 325
#define BACKTICK 326
#define ASSIGN 327
#define SUB_ASSIGN 328
#define ADD_ASSIGN 329
#define MOD_ASSIGN 330
#define DIV_ASSIGN 331
#define MUL_ASSIGN 332
#define OR_OP 333
#define AND_OP 334
#define NCLOSE 335
#define NOPEN 336
#define LOWER_THAN_ELSE 337
#define ELSE 338
#define LOWER_THAN_DEREF 339




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
#include "snippetGen.h"
#include <sstream>

extern "C" {
//   std::string lineStr;
   void yyerror(char *s); 
   void yyerrorNonUni(char *s);
   void yyerrorNoTok(char *s);
   void yyerrorNoTokNonUni(char *s);
   void yywarn(char *s);
   int yyparse();
   void makeOneTimeStatement(BPatch_snippet &statement);
   void makeOneTimeStatementGbl(BPatch_snippet &statement);
   void getErrorBase(char *errbase, int length);
//   char *yytext;
//   char *dynCSnippetName;
}

extern std::string lineStr;


//name of current snippet for error reporting
char *dynCSnippetName = "";
SnippetGenerator *snippetGen;
BPatch_point *snippetPoint = NULL;

std::set<std::string> *universalErrors = new std::set<std::string>();

//void yywarn(char *s);

int oneTimeCount = 0;
int oneTimeGblCount = 0;
int yylex();

BPatch_snippet *parse_result;

bool fatalError = false;
bool actionTaken = false;

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
/* Line 193 of yacc.c.  */
#line 370 "dynC.tab.c"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
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


/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 395 "dynC.tab.c"

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
	 || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
	     && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss;
  YYSTYPE yyvs;
    YYLTYPE yyls;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE) + sizeof (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

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
#define YYFINAL  47
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   479

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  102
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  17
/* YYNRULES -- Number of rules.  */
#define YYNRULES  80
/* YYNRULES -- Number of states.  */
#define YYNSTATES  158

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   339

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    78,     2,     2,     2,    70,    80,     2,
      98,    99,    68,    66,     2,    67,     2,    69,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     100,     2,   101,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    94,     2,    95,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    96,    71,    97,    79,     2,     2,     2,
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
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    72,    73,    74,    75,    76,    77,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     5,     8,    13,    19,    28,    38,    40,
      43,    45,    48,    52,    57,    59,    61,    64,    67,    73,
      81,    83,    87,    94,   101,   102,   104,   108,   110,   112,
     114,   118,   122,   126,   130,   134,   138,   142,   146,   148,
     150,   152,   154,   157,   160,   164,   169,   174,   178,   183,
     188,   192,   197,   202,   207,   209,   211,   213,   217,   219,
     221,   223,   227,   231,   233,   237,   241,   245,   249,   253,
     257,   261,   265,   269,   273,   277,   281,   283,   286,   289,
     292
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
     103,     0,    -1,   106,    -1,   105,     4,    -1,   105,     4,
      81,   117,    -1,   105,     4,    94,     8,    95,    -1,   105,
       4,    94,    95,    81,    96,   116,    97,    -1,   105,     4,
      94,     8,    95,    81,    96,   116,    97,    -1,     7,    -1,
      27,   105,    -1,   107,    -1,   106,   107,    -1,    90,   106,
      89,    -1,   106,    90,   106,    89,    -1,     1,    -1,     9,
      -1,   104,    73,    -1,   117,    73,    -1,    42,    98,   112,
      99,   108,    -1,    42,    98,   112,    99,   108,    92,   108,
      -1,   107,    -1,    96,   106,    97,    -1,    47,    77,     4,
      98,   110,    99,    -1,    46,    77,     4,    98,   110,    99,
      -1,    -1,   117,    -1,   110,    55,   117,    -1,    12,    -1,
      13,    -1,   111,    -1,   117,   100,   117,    -1,   117,   101,
     117,    -1,   117,    63,   117,    -1,   117,    65,   117,    -1,
     117,    64,   117,    -1,   117,    62,   117,    -1,   112,    61,
     112,    -1,   112,    60,   112,    -1,    45,    -1,    43,    -1,
      44,    -1,     4,    -1,    68,     4,    -1,    80,     4,    -1,
      46,    77,     4,    -1,    68,    46,    77,     4,    -1,    80,
      46,    77,     4,    -1,   113,    77,     4,    -1,    68,   113,
      77,     4,    -1,    80,   113,    77,     4,    -1,   113,    77,
       8,    -1,    68,   113,    77,     8,    -1,    80,   113,    77,
       8,    -1,   114,    94,   117,    95,    -1,     8,    -1,     6,
      -1,   115,    -1,   116,    55,   115,    -1,   114,    -1,   115,
      -1,    54,    -1,    47,    77,     4,    -1,   117,    68,   117,
      -1,   109,    -1,   114,    81,   117,    -1,   114,    83,   117,
      -1,   114,    82,   117,    -1,   114,    86,   117,    -1,   114,
      85,   117,    -1,   114,    84,   117,    -1,   117,    69,   117,
      -1,   117,    70,   117,    -1,   117,    66,   117,    -1,   117,
      67,   117,    -1,   117,    71,   117,    -1,    98,   117,    99,
      -1,   118,    -1,   114,    15,    -1,    15,   114,    -1,   114,
      16,    -1,    16,   114,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   157,   157,   172,   195,   215,   251,   290,   346,   351,
     423,   429,   434,   442,   452,   457,   463,   468,   479,   487,
     498,   500,   507,   524,   544,   549,   555,   564,   568,   575,
     576,   581,   586,   591,   596,   601,   606,   611,   621,   627,
     633,   654,   665,   677,   690,   701,   713,   727,   756,   783,
     810,   831,   853,   877,   905,   911,   920,   926,   932,   933,
     934,   935,  1009,  1015,  1020,  1026,  1032,  1038,  1044,  1050,
    1056,  1062,  1068,  1074,  1080,  1089,  1090,  1099,  1105,  1111,
    1117
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "KNOWN_ERROR_TOK", "IDENTIFIER",
  "CONSTANT", "STRING", "TYPE", "NUMBER", "ERROR", "EOL", "SIZEOF",
  "D_TRUE", "D_FALSE", "PTR_OP", "INC_OP", "DEC_OP", "LEFT_OP", "RIGHT_OP",
  "LEFT_ASSIGN", "RIGHT_ASSIGN", "AND_ASSIGN", "XOR_ASSIGN", "OR_ASSIGN",
  "TYPE_NAME", "TYPEDEF", "EXTERN", "STATIC", "CHAR", "SHORT", "INT",
  "LONG", "SIGNED", "UNSIGNED", "FLOAT", "DOUBLE", "CONST", "VOID",
  "STRUCT", "UNION", "ENUM", "ELLIPSIS", "IF", "LOCAL", "PARAM", "GLOBAL",
  "INF", "DYNINST", "INST", "NEWLINE", "CASE", "DEFAULT", "SWITCH",
  "RETURN", "NILL", "COMMA", "AMPERSAND", "ASTERISK", "DOT", "NOT", "OR",
  "AND", "NOT_EQ", "EQ", "GREATER_EQ", "LESS_EQ", "'+'", "'-'", "'*'",
  "'/'", "'%'", "'|'", "COLON", "SEMI", "END_BLOCK", "START_BLOCK",
  "DOLLAR", "BACKTICK", "'!'", "'~'", "'&'", "ASSIGN", "SUB_ASSIGN",
  "ADD_ASSIGN", "MOD_ASSIGN", "DIV_ASSIGN", "MUL_ASSIGN", "OR_OP",
  "AND_OP", "NCLOSE", "NOPEN", "LOWER_THAN_ELSE", "ELSE",
  "LOWER_THAN_DEREF", "'['", "']'", "'{'", "'}'", "'('", "')'", "'<'",
  "'>'", "$accept", "start", "var_declaration", "var_decl_modifiers",
  "statement_list", "statement", "block", "func_call", "param_list",
  "bool_constant", "bool_expression", "var_modifiers", "variable_expr",
  "constant", "const_list", "arith_expression", "inc_decr_expr", 0
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
     315,   316,   317,   318,   319,   320,    43,    45,    42,    47,
      37,   124,   321,   322,   323,   324,   325,   326,    33,   126,
      38,   327,   328,   329,   330,   331,   332,   333,   334,   335,
     336,   337,   338,   339,    91,    93,   123,   125,    40,    41,
      60,    62
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,   102,   103,   104,   104,   104,   104,   104,   105,   105,
     106,   106,   106,   106,   107,   107,   107,   107,   107,   107,
     108,   108,   109,   109,   110,   110,   110,   111,   111,   112,
     112,   112,   112,   112,   112,   112,   112,   112,   113,   113,
     113,   114,   114,   114,   114,   114,   114,   114,   114,   114,
     114,   114,   114,   114,   115,   115,   116,   116,   117,   117,
     117,   117,   117,   117,   117,   117,   117,   117,   117,   117,
     117,   117,   117,   117,   117,   117,   117,   118,   118,   118,
     118
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     2,     4,     5,     8,     9,     1,     2,
       1,     2,     3,     4,     1,     1,     2,     2,     5,     7,
       1,     3,     6,     6,     0,     1,     3,     1,     1,     1,
       3,     3,     3,     3,     3,     3,     3,     3,     1,     1,
       1,     1,     2,     2,     3,     4,     4,     3,     4,     4,
       3,     4,     4,     4,     1,     1,     1,     3,     1,     1,
       1,     3,     3,     1,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     1,     2,     2,     2,
       2
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,    14,    41,    55,     8,    54,    15,     0,     0,     0,
       0,    39,    40,    38,     0,     0,    60,     0,     0,     0,
       0,     0,     0,     0,     0,    10,    63,     0,    58,    59,
       0,    76,     0,    78,    80,     9,     0,     0,     0,    42,
       0,     0,    43,     0,     0,     0,     0,     1,    16,     3,
       0,    11,     0,    77,    79,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    17,     0,
      27,    28,    29,     0,     0,    44,    61,     0,     0,     0,
       0,    12,    75,     0,     0,     0,    47,    50,    64,    66,
      65,    69,    68,    67,     0,    72,    73,    62,    70,    71,
      74,    44,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    24,    24,    45,    48,    51,    46,    49,    52,     4,
       0,     0,    13,    53,    37,    36,     0,    20,    18,    35,
      32,    34,    33,    30,    31,     0,    25,     0,     5,     0,
       0,     0,     0,    23,    22,     0,     0,    21,    19,    26,
       0,    56,     0,     0,     0,     6,     7,    57
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    21,    22,    23,    24,    25,   128,    26,   135,    72,
      73,    27,    28,    29,   152,    30,    31
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -86
static const yytype_int16 yypact[] =
{
     266,   -86,   -86,   -86,   -86,   -86,   -86,    99,    99,    -4,
     -67,   -86,   -86,   -86,   -64,    -7,   -86,   107,   111,   266,
     381,    77,    29,   100,    51,   -86,   -86,    30,   -10,   -86,
     123,   -86,    31,    12,    12,   -86,   332,   106,   108,   -86,
      36,    37,   -86,    39,    44,   119,   135,   -86,   -86,   -70,
     266,   -86,     6,   -86,   -86,   381,   381,   381,   381,   381,
     381,   381,   381,   381,   381,   381,   381,   381,   -86,   120,
     -86,   -86,   -86,   -43,   339,    32,    34,   125,    11,   129,
      60,   -86,   -86,   381,     4,   168,   -86,   -86,   -86,   -86,
     -86,   -86,   -86,   -86,   -41,    68,    68,    69,    69,    69,
     -86,   -86,   332,   332,   315,   381,   381,   381,   381,   381,
     381,   381,   381,   -86,   -86,   -86,   -86,   -86,   -86,   171,
      52,    67,   -86,   -86,    97,   -86,   266,   -86,    76,   171,
     171,   171,   171,   171,   171,   -51,   171,   -46,    78,    64,
     217,   315,   381,   -86,   -86,    74,    26,   -86,   -86,   171,
      26,   -86,   -48,   -47,    26,   -86,   -86,   -86
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
     -86,   -86,   -86,   162,   -17,   -23,    40,   -86,    66,   -86,
     -82,    62,    93,   -85,    35,   -20,   -86
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -3
static const yytype_int16 yytable[] =
{
      46,    51,    45,     4,   142,    53,    54,   154,   154,   142,
      86,    83,   120,    37,    87,   114,    74,   102,   103,   115,
     124,   125,    51,     9,    84,    62,    63,    64,    65,    66,
      67,    36,     3,    85,     5,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   143,   155,
     156,    -2,     1,   144,   123,     2,   104,     3,     4,     5,
       6,   151,    51,   119,   117,   151,     7,     8,   118,   157,
      38,    55,    56,    57,    58,    59,    60,    47,     9,    41,
      44,   127,    74,    74,    61,   129,   130,   131,   132,   133,
     134,   136,   136,    10,    11,    12,    13,    14,    15,   121,
      33,    34,    48,     2,    49,    16,    61,    52,    69,   140,
      75,    39,    76,    77,    78,    42,    79,    51,   127,    17,
       1,    80,   149,     2,   101,     3,     4,     5,     6,   113,
     111,    18,   112,   116,     7,     8,    64,    65,    66,    67,
      67,    50,    11,    12,    13,    32,     9,   138,   139,    20,
      11,    12,    13,    40,    11,    12,    13,    43,   103,   145,
     146,    10,    11,    12,    13,    14,    15,    17,   141,     1,
     150,    35,     2,    16,     3,     4,     5,     6,   137,    18,
       0,   148,     0,     7,     8,   153,     0,    17,     0,    62,
      63,    64,    65,    66,    67,     9,    68,     0,     0,    18,
       0,    62,    63,    64,    65,    66,    67,     0,    81,    50,
      10,    11,    12,    13,    14,    15,     0,    20,     1,     0,
       0,     2,    16,     3,     4,     5,     6,     0,     0,     0,
       0,     0,     7,     8,    82,     0,    17,    62,    63,    64,
      65,    66,    67,     0,     9,     0,     0,     0,    18,     0,
       0,     0,     0,     0,     0,     0,     0,   122,    50,    10,
      11,    12,    13,    14,    15,     0,    20,     1,     0,     0,
       2,    16,     3,     4,     5,     6,     0,     0,     0,     0,
       0,     7,     8,     0,     0,    17,     0,     0,     0,     0,
       0,     0,     0,     9,     0,     0,     0,    18,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    50,    10,    11,
      12,    13,    14,    15,   147,    20,     1,     0,     0,     2,
      16,     3,     4,     5,     6,     0,     0,     0,     0,     0,
       7,     8,     0,     0,    17,     0,     2,     0,     3,     0,
       5,     0,     9,     0,    70,    71,    18,     7,     8,     0,
       0,     0,     0,     0,     0,     0,    19,    10,    11,    12,
      13,    14,    15,     0,    20,     0,     0,     0,     0,    16,
       0,     0,     0,     0,     0,    11,    12,    13,    14,    15,
       0,     0,     0,    17,     0,     2,    16,     3,     0,     5,
       0,     0,     0,     0,     0,    18,     7,     8,     0,     0,
      17,   105,   106,   107,   108,    62,    63,    64,    65,    66,
      67,   126,    18,    20,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,    13,    14,    15,     0,
      20,     0,     0,     0,     0,    16,     0,     0,     0,   109,
     110,     0,     0,     0,     0,     0,     0,     0,     0,    17,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    18,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    20
};

static const yytype_int16 yycheck[] =
{
      20,    24,    19,     7,    55,    15,    16,    55,    55,    55,
       4,    81,     8,    77,     8,     4,    36,    60,    61,     8,
     102,   103,    45,    27,    94,    66,    67,    68,    69,    70,
      71,    98,     6,    50,     8,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    99,    97,
      97,     0,     1,    99,    95,     4,    99,     6,     7,     8,
       9,   146,    85,    83,     4,   150,    15,    16,     8,   154,
      77,    81,    82,    83,    84,    85,    86,     0,    27,    17,
      18,   104,   102,   103,    94,   105,   106,   107,   108,   109,
     110,   111,   112,    42,    43,    44,    45,    46,    47,    95,
       7,     8,    73,     4,     4,    54,    94,    77,    77,   126,
       4,     4,     4,    77,    77,     4,    77,   140,   141,    68,
       1,    77,   142,     4,     4,     6,     7,     8,     9,     4,
      98,    80,    98,     4,    15,    16,    68,    69,    70,    71,
      71,    90,    43,    44,    45,    46,    27,    95,    81,    98,
      43,    44,    45,    46,    43,    44,    45,    46,    61,    81,
      96,    42,    43,    44,    45,    46,    47,    68,    92,     1,
      96,     9,     4,    54,     6,     7,     8,     9,   112,    80,
      -1,   141,    -1,    15,    16,   150,    -1,    68,    -1,    66,
      67,    68,    69,    70,    71,    27,    73,    -1,    -1,    80,
      -1,    66,    67,    68,    69,    70,    71,    -1,    89,    90,
      42,    43,    44,    45,    46,    47,    -1,    98,     1,    -1,
      -1,     4,    54,     6,     7,     8,     9,    -1,    -1,    -1,
      -1,    -1,    15,    16,    99,    -1,    68,    66,    67,    68,
      69,    70,    71,    -1,    27,    -1,    -1,    -1,    80,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    89,    90,    42,
      43,    44,    45,    46,    47,    -1,    98,     1,    -1,    -1,
       4,    54,     6,     7,     8,     9,    -1,    -1,    -1,    -1,
      -1,    15,    16,    -1,    -1,    68,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    -1,    -1,    -1,    80,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    90,    42,    43,
      44,    45,    46,    47,    97,    98,     1,    -1,    -1,     4,
      54,     6,     7,     8,     9,    -1,    -1,    -1,    -1,    -1,
      15,    16,    -1,    -1,    68,    -1,     4,    -1,     6,    -1,
       8,    -1,    27,    -1,    12,    13,    80,    15,    16,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    90,    42,    43,    44,
      45,    46,    47,    -1,    98,    -1,    -1,    -1,    -1,    54,
      -1,    -1,    -1,    -1,    -1,    43,    44,    45,    46,    47,
      -1,    -1,    -1,    68,    -1,     4,    54,     6,    -1,     8,
      -1,    -1,    -1,    -1,    -1,    80,    15,    16,    -1,    -1,
      68,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    96,    80,    98,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    43,    44,    45,    46,    47,    -1,
      98,    -1,    -1,    -1,    -1,    54,    -1,    -1,    -1,   100,
     101,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,     4,     6,     7,     8,     9,    15,    16,    27,
      42,    43,    44,    45,    46,    47,    54,    68,    80,    90,
      98,   103,   104,   105,   106,   107,   109,   113,   114,   115,
     117,   118,    46,   114,   114,   105,    98,    77,    77,     4,
      46,   113,     4,    46,   113,   106,   117,     0,    73,     4,
      90,   107,    77,    15,    16,    81,    82,    83,    84,    85,
      86,    94,    66,    67,    68,    69,    70,    71,    73,    77,
      12,    13,   111,   112,   117,     4,     4,    77,    77,    77,
      77,    89,    99,    81,    94,   106,     4,     8,   117,   117,
     117,   117,   117,   117,   117,   117,   117,   117,   117,   117,
     117,     4,    60,    61,    99,    62,    63,    64,    65,   100,
     101,    98,    98,     4,     4,     8,     4,     4,     8,   117,
       8,    95,    89,    95,   112,   112,    96,   107,   108,   117,
     117,   117,   117,   117,   117,   110,   117,   110,    95,    81,
     106,    92,    55,    99,    99,    81,    96,    97,   108,   117,
      96,   115,   116,   116,    55,    97,    97,   115
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
		  Type, Value, Location); \
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
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    YYLTYPE const * const yylocationp;
#endif
{
  if (!yyvaluep)
    return;
  YYUSE (yylocationp);
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
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep, yylocationp)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    YYLTYPE const * const yylocationp;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  YY_LOCATION_PRINT (yyoutput, *yylocationp);
  YYFPRINTF (yyoutput, ": ");
  yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp);
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
yy_reduce_print (YYSTYPE *yyvsp, YYLTYPE *yylsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yylsp, yyrule)
    YYSTYPE *yyvsp;
    YYLTYPE *yylsp;
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
		       , &(yylsp[(yyi + 1) - (yynrhs)])		       );
      fprintf (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, yylsp, Rule); \
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
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, YYLTYPE *yylocationp)
#else
static void
yydestruct (yymsg, yytype, yyvaluep, yylocationp)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
    YYLTYPE *yylocationp;
#endif
{
  YYUSE (yyvaluep);
  YYUSE (yylocationp);

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
/* Location data for the look-ahead symbol.  */
YYLTYPE yylloc;



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

  /* The location stack.  */
  YYLTYPE yylsa[YYINITDEPTH];
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;
  /* The locations where the error started and ended.  */
  YYLTYPE yyerror_range[2];

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

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
  yylsp = yyls;
#if YYLTYPE_IS_TRIVIAL
  /* Initialize the default location before parsing starts.  */
  yylloc.first_line   = yylloc.last_line   = 1;
  yylloc.first_column = yylloc.last_column = 0;
#endif

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
	YYLTYPE *yyls1 = yyls;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yyls1, yysize * sizeof (*yylsp),
		    &yystacksize);
	yyls = yyls1;
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
	YYSTACK_RELOCATE (yyls);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

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
  *++yylsp = yylloc;
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

  /* Default location.  */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 158 "../src/C.y"
    { 
       oneTimeCount = 0;
       oneTimeGblCount = 0;
       (yyvsp[(1) - (1)].snippetList)->insert((yyvsp[(1) - (1)].snippetList)->end(), endSnippets.begin(), endSnippets.end());
       parse_result = new BPatch_sequence(*(yyvsp[(1) - (1)].snippetList)); 
       delete (yyvsp[(1) - (1)].snippetList);
       endSnippets.clear();
       if(verbose) {
          printf("\n");
          fflush(stdout);
       }
    ;}
    break;

  case 3:
#line 173 "../src/C.y"
    {
       //IDENTIFIER leaks, but how to fix b/c use of $0?
       std::string mangledName;
       mangledName = dynC_API::mangle((yyvsp[(2) - (2)].sval), dynCSnippetName, (yyvsp[(1) - (2)].varSpec).type);
       if(verbose) printf("name : %s\n", mangledName.c_str());
       (yyval.snippet) = snippetGen->findOrCreateVariable(mangledName.c_str(), (yyvsp[(1) - (2)].varSpec).type);
       if((yyval.snippet) == NULL){
          (yyval.snippet) = new BPatch_nullExpr();
          char *errString = strdup(snippetGen->getError().c_str());
          yyerror(errString);
          free(errString);
          break;
       }

        if(!((yyvsp[(1) - (2)].varSpec).isStatic || (yyvsp[(1) - (2)].varSpec).isGlobal)){ 
          BPatch_snippet setSn = BPatch_constExpr(0); // if int etc.
          if(strcmp((yyvsp[(1) - (2)].varSpec).type, "char *") == 0){
             setSn = BPatch_constExpr("");
          }
          endSnippets.push_back(new BPatch_arithExpr(BPatch_assign, *(yyval.snippet), setSn));
          }
    ;}
    break;

  case 4:
#line 196 "../src/C.y"
    {   
      
       std::string mangledName = dynC_API::mangle((yyvsp[(2) - (4)].sval), dynCSnippetName, (yyvsp[(1) - (4)].varSpec).type);
       if(verbose) printf("name : %s\n", mangledName.c_str());
       BPatch_snippet *alloc = snippetGen->findOrCreateVariable(mangledName.c_str(), (yyvsp[(1) - (4)].varSpec).type);
       if(alloc == NULL){
          (yyval.snippet) = new BPatch_nullExpr();
          char *errString = strdup(snippetGen->getError().c_str());
          yyerror(errString);
          free(errString);
          break;
       }
       BPatch_arithExpr *assign = new BPatch_arithExpr(BPatch_assign, *alloc, *(yyvsp[(4) - (4)].snippet));
       (yyval.snippet) = assign;

       if((yyvsp[(1) - (4)].varSpec).isStatic || (yyvsp[(1) - (4)].varSpec).isGlobal){
          makeOneTimeStatementGbl(*(yyval.snippet));
       }       
    ;}
    break;

  case 5:
#line 216 "../src/C.y"
    {
       //IDENTIFIER leaks, but how to fix b/c use of $0?
       if((yyvsp[(4) - (5)].ival) < 0){
          std::stringstream errMessage;
          errMessage << "Invalid array size: " << (yyvsp[(4) - (5)].ival);
          char *eM = strdup(errMessage.str().c_str());
          yyerrorNoTok(eM);
          free(eM);
          (yyval.snippet) = new BPatch_nullExpr();
          break;
       }
      
       std::stringstream type;
       type << (yyvsp[(1) - (5)].varSpec).type << "[" << (yyvsp[(4) - (5)].ival) << "]";
       std::string mangledName = dynC_API::mangle((yyvsp[(2) - (5)].sval), dynCSnippetName, type.str().c_str());
      
       (yyval.snippet) = snippetGen->findOrCreateArray(mangledName.c_str(), (yyvsp[(1) - (5)].varSpec).type, (yyvsp[(4) - (5)].ival));
       if((yyval.snippet) == NULL){
          (yyval.snippet) = new BPatch_nullExpr();
          char *errString = strdup(snippetGen->getError().c_str());
          yyerror(errString);
          free(errString);
          break;
       }
       if(!((yyvsp[(1) - (5)].varSpec).isStatic || (yyvsp[(1) - (5)].varSpec).isGlobal)){ 
          BPatch_snippet setSn = BPatch_constExpr(0); // if int etc.
          if(strcmp((yyvsp[(1) - (5)].varSpec).type, "char *") == 0){
             setSn = BPatch_constExpr("");
          }
          for(int n = 0; n < (yyvsp[(4) - (5)].ival); ++n){
             endSnippets.push_back(new BPatch_arithExpr(BPatch_assign, BPatch_arithExpr(BPatch_ref, *(yyval.snippet), BPatch_constExpr(n)), setSn));             
          }
       }

    ;}
    break;

  case 6:
#line 252 "../src/C.y"
    {
       //IDENTIFIER leaks, but how to fix b/c use of $0?
       std::stringstream type;
       type << (yyvsp[(1) - (8)].varSpec).type << "[" << (yyvsp[(7) - (8)].snippetStringListPair)->size() << "]";
       std::string mangledName = dynC_API::mangle((yyvsp[(2) - (8)].sval), dynCSnippetName, type.str().c_str());

       (yyval.snippet) = snippetGen->findOrCreateArray(mangledName.c_str(), (yyvsp[(1) - (8)].varSpec).type, (yyvsp[(7) - (8)].snippetStringListPair)->size()); //will only allocate memory if nessessary
       if((yyval.snippet) == NULL){
          (yyval.snippet) = new BPatch_nullExpr();
          char *errString = strdup(snippetGen->getError().c_str());
          yyerror(errString);
          free(errString);
          break;
       }
       std::vector<BPatch_snippet* > *assignVect = new std::vector<BPatch_snippet *>();
       assignVect->push_back((yyval.snippet));
       for(unsigned int n = 0; n < (yyvsp[(7) - (8)].snippetStringListPair)->size(); ++n){
          if(strcmp((yyvsp[(1) - (8)].varSpec).type, (*(yyvsp[(7) - (8)].snippetStringListPair))[n].second) != 0){
             std::string errMessage = "Type conflict when trying to initialize array of type \'";
             errMessage += type.str();
             errMessage += "\' with a value of type ";
             errMessage += (*(yyvsp[(7) - (8)].snippetStringListPair))[n].second;
             char *eM = strdup(errMessage.c_str());
             yyerrorNoTok(eM);
             free(eM);
             (yyval.snippet) = new BPatch_nullExpr();
             break;
          }
          BPatch_snippet *assign = new BPatch_arithExpr(BPatch_assign, BPatch_arithExpr(BPatch_ref, *(yyval.snippet), BPatch_constExpr(n)), *(*(yyvsp[(7) - (8)].snippetStringListPair))[n].first);      
          
          if((yyvsp[(1) - (8)].varSpec).isStatic || (yyvsp[(1) - (8)].varSpec).isGlobal){
              makeOneTimeStatement(*assign);
              }
          //makeOneTimeStatement(*assign);
          assignVect->push_back(assign);
       }
       (yyval.snippet) = new BPatch_sequence(*assignVect);
    ;}
    break;

  case 7:
#line 291 "../src/C.y"
    {
       //IDENTIFIER leaks, but how to fix b/c use of $0?
       std::vector<BPatch_snippet *> argVect;
       std::stringstream type;
       if((yyvsp[(4) - (9)].ival) < 0){
          std::stringstream errMessage;
          errMessage << "Invalid array size: " << (yyvsp[(4) - (9)].ival);
          char *eM = strdup(errMessage.str().c_str());
          yyerrorNoTok(eM);
          free(eM);
          (yyval.snippet) = new BPatch_nullExpr();
          break;
       }
       type << (yyvsp[(1) - (9)].varSpec).type << "[" << (yyvsp[(4) - (9)].ival) << "]";
       std::string mangledName = dynC_API::mangle((yyvsp[(2) - (9)].sval), dynCSnippetName, type.str().c_str());

       (yyval.snippet) = snippetGen->findOrCreateArray(mangledName.c_str(), (yyvsp[(1) - (9)].varSpec).type, (yyvsp[(4) - (9)].ival)); //will only allocate memory if nessessary
       if((yyval.snippet) == NULL){
          (yyval.snippet) = new BPatch_nullExpr();
          char *errString = strdup(snippetGen->getError().c_str());
          yyerror(errString);
          free(errString);
          break;
       }
       std::vector<BPatch_snippet* > *assignVect = new std::vector<BPatch_snippet *>();
       assignVect->push_back((yyval.snippet));
       if((unsigned int)(yyvsp[(4) - (9)].ival) != (yyvsp[(8) - (9)].snippetStringListPair)->size()){
          yyerrorNoTok("Invalid number of arguments given in array initialization");
          (yyval.snippet) = new BPatch_nullExpr();
          break;
       }
       for(int n = 0; n < (yyvsp[(4) - (9)].ival); ++n){
          if(strcmp((yyvsp[(1) - (9)].varSpec).type, (*(yyvsp[(8) - (9)].snippetStringListPair))[n].second) != 0){
             std::string errMessage = "Type conflict when trying to initialize array of type \'";
             errMessage += type.str();
             errMessage += "\' with a value of type ";
             errMessage += (*(yyvsp[(8) - (9)].snippetStringListPair))[n].second;
             char *eM = strdup(errMessage.c_str());
             yyerrorNoTok(eM);
             free(eM);
             (yyval.snippet) = new BPatch_nullExpr();
             break;
          }
          BPatch_snippet *assign = new BPatch_arithExpr(BPatch_assign, BPatch_arithExpr(BPatch_ref, *(yyval.snippet), BPatch_constExpr(n)), *(*(yyvsp[(8) - (9)].snippetStringListPair))[n].first);      
          if((yyvsp[(1) - (9)].varSpec).isStatic || (yyvsp[(1) - (9)].varSpec).isGlobal){
             makeOneTimeStatement(*assign);
          }
          //   makeOneTimeStatement(*assign);
          assignVect->push_back(assign);
       
       }
       (yyval.snippet) = new BPatch_sequence(*assignVect);
    ;}
    break;

  case 8:
#line 347 "../src/C.y"
    {
      YYSTYPE::VariableSpec rSpec = {false,false,false,false,false,false,false,false,(yyvsp[(1) - (1)].sval)};
      (yyval.varSpec) = rSpec;
   ;}
    break;

  case 9:
#line 352 "../src/C.y"
    {
      if ((yyvsp[(2) - (2)].varSpec).isStatic){
         //throw error: two static
         yyerror("Syntax error");
      }else{
         (yyvsp[(2) - (2)].varSpec).isStatic = true;
      }
      (yyval.varSpec) = (yyvsp[(2) - (2)].varSpec);
  ;}
    break;

  case 10:
#line 424 "../src/C.y"
    { 
       if(verbose) printf("\n");
       (yyval.snippetList) = new BPatch_Vector<BPatch_snippet *>; 
       (yyval.snippetList)->push_back((yyvsp[(1) - (1)].snippet));
    ;}
    break;

  case 11:
#line 430 "../src/C.y"
    {
       (yyvsp[(1) - (2)].snippetList)->push_back((yyvsp[(2) - (2)].snippet));
       (yyval.snippetList) = (yyvsp[(1) - (2)].snippetList);
    ;}
    break;

  case 12:
#line 435 "../src/C.y"
    {
       BPatch_sequence *seq = new BPatch_sequence(*(yyvsp[(2) - (3)].snippetList));
       makeOneTimeStatementGbl(*seq);
       std::vector<BPatch_snippet *> *retVect = new std::vector<BPatch_snippet *>;
       retVect->push_back(seq);
       (yyval.snippetList) = retVect;       
    ;}
    break;

  case 13:
#line 443 "../src/C.y"
    {
       BPatch_sequence seq = BPatch_sequence(*(yyvsp[(3) - (4)].snippetList));
       makeOneTimeStatementGbl(seq);
       (yyvsp[(1) - (4)].snippetList)->push_back(&seq);
       (yyval.snippetList) = (yyvsp[(1) - (4)].snippetList);
    ;}
    break;

  case 14:
#line 453 "../src/C.y"
    {
       (yyval.snippet) = new BPatch_nullExpr();
       actionTaken = false;
    ;}
    break;

  case 15:
#line 458 "../src/C.y"
    {
       yyerrorNoTok((yyvsp[(1) - (1)].context));
       (yyval.snippet) = new BPatch_nullExpr();
       actionTaken = false;
    ;}
    break;

  case 16:
#line 464 "../src/C.y"
    {
       (yyval.snippet) = (yyvsp[(1) - (2)].snippet);
    ;}
    break;

  case 17:
#line 469 "../src/C.y"
    {
       if(!actionTaken){
          yywarn("Statement does nothing!");
          (yyval.snippet) = new BPatch_nullExpr();
       }else{
          (yyval.snippet) = (yyvsp[(1) - (2)].snippet);
       } 
       actionTaken = false;
    ;}
    break;

  case 18:
#line 480 "../src/C.y"
    {
       if(verbose) printf(" if () ");
       (yyval.snippet) = new BPatch_ifExpr(*(yyvsp[(3) - (5)].boolExpr), *(yyvsp[(5) - (5)].snippet));
       delete (yyvsp[(3) - (5)].boolExpr);
       delete (yyvsp[(5) - (5)].snippet);
    ;}
    break;

  case 19:
#line 488 "../src/C.y"
    {
       if(verbose) printf(" if () else ");
       (yyval.snippet) = new BPatch_ifExpr(*(yyvsp[(3) - (7)].boolExpr), *(yyvsp[(5) - (7)].snippet), *(yyvsp[(7) - (7)].snippet));
       delete (yyvsp[(3) - (7)].boolExpr);
       delete (yyvsp[(5) - (7)].snippet);
       delete (yyvsp[(7) - (7)].snippet);
    ;}
    break;

  case 21:
#line 501 "../src/C.y"
    {
        (yyval.snippet) = new BPatch_sequence(*(yyvsp[(2) - (3)].snippetList));
        delete (yyvsp[(2) - (3)].snippetList);
     ;}
    break;

  case 22:
#line 508 "../src/C.y"
    {
       //TODO: built in dyninst actions and (future) other snippet calls?
       if(strcmp((yyvsp[(3) - (6)].sval), "break") == 0){
          if(verbose) printf("break_ ()");
          (yyval.snippet) = new BPatch_breakPointExpr();
       }else if(strcmp((yyvsp[(3) - (6)].sval), "stopThread") == 0){
          //how?
          if(verbose) printf("stopThread ()");
          (yyval.snippet) = new BPatch_nullExpr();
       }else{
          yyerror("Dyninst function not found!\n");
          (yyval.snippet) = new BPatch_nullExpr();
       }
       delete (yyvsp[(3) - (6)].sval);
    ;}
    break;

  case 23:
#line 526 "../src/C.y"
    { 
       BPatch_function *func = snippetGen->findFunction((yyvsp[(3) - (6)].sval), *(yyvsp[(5) - (6)].snippetList));
       if(func == NULL){
          (yyval.snippet) = new BPatch_nullExpr();
          char *errString = strdup(snippetGen->getError().c_str());
          yyerrorNoTok(errString);
          free(errString);
          break;
       }
       if((yyvsp[(5) - (6)].snippetList) == NULL){
          fprintf(stderr, "Internal Error: Param_list is null!\n");
       }
       (yyval.snippet) = new BPatch_funcCallExpr(*func, *(yyvsp[(5) - (6)].snippetList));
    ;}
    break;

  case 24:
#line 544 "../src/C.y"
    {
       //No parameters, return an empty vector
       (yyval.snippetList) = new BPatch_Vector<BPatch_snippet *>;
    ;}
    break;

  case 25:
#line 550 "../src/C.y"
    { 
       (yyval.snippetList) = new BPatch_Vector<BPatch_snippet *>; 
       (yyval.snippetList)->push_back((yyvsp[(1) - (1)].snippet));
    ;}
    break;

  case 26:
#line 556 "../src/C.y"
    { 
       if(verbose) printf(" , ");
       (yyvsp[(1) - (3)].snippetList)->push_back((yyvsp[(3) - (3)].snippet)); 
       (yyval.snippetList) = (yyvsp[(1) - (3)].snippetList);
    ;}
    break;

  case 27:
#line 565 "../src/C.y"
    { 
       (yyval.boolExpr) = new BPatch_boolExpr(BPatch_eq, BPatch_constExpr(0), BPatch_constExpr(0));
    ;}
    break;

  case 28:
#line 569 "../src/C.y"
    {
       (yyval.boolExpr) = new BPatch_boolExpr(BPatch_ne, BPatch_constExpr(0), BPatch_constExpr(0));
    ;}
    break;

  case 30:
#line 577 "../src/C.y"
    {
       if(verbose) printf(" < ");
       (yyval.boolExpr) = new BPatch_boolExpr(BPatch_lt, *(yyvsp[(1) - (3)].snippet), *(yyvsp[(3) - (3)].snippet));
    ;}
    break;

  case 31:
#line 582 "../src/C.y"
    {
       if(verbose) printf(" > ");
       (yyval.boolExpr) = new BPatch_boolExpr(BPatch_gt, *(yyvsp[(1) - (3)].snippet), *(yyvsp[(3) - (3)].snippet));
    ;}
    break;

  case 32:
#line 587 "../src/C.y"
    {
       if(verbose) printf(" == ");
       (yyval.boolExpr) = new BPatch_boolExpr(BPatch_eq, *(yyvsp[(1) - (3)].snippet), *(yyvsp[(3) - (3)].snippet));
    ;}
    break;

  case 33:
#line 592 "../src/C.y"
    {
       if(verbose) printf(" <= ");
       (yyval.boolExpr) = new BPatch_boolExpr(BPatch_le, *(yyvsp[(1) - (3)].snippet), *(yyvsp[(3) - (3)].snippet));
    ;}
    break;

  case 34:
#line 597 "../src/C.y"
    {
       if(verbose) printf(" >= ");
       (yyval.boolExpr) = new BPatch_boolExpr(BPatch_ge, *(yyvsp[(1) - (3)].snippet), *(yyvsp[(3) - (3)].snippet));
    ;}
    break;

  case 35:
#line 602 "../src/C.y"
    {
       if(verbose) printf(" != ");
       (yyval.boolExpr) = new BPatch_boolExpr(BPatch_ne, *(yyvsp[(1) - (3)].snippet), *(yyvsp[(3) - (3)].snippet));
    ;}
    break;

  case 36:
#line 607 "../src/C.y"
    {
       if(verbose) printf(" AND ");
       (yyval.boolExpr) = new BPatch_boolExpr(BPatch_and, *(yyvsp[(1) - (3)].boolExpr), *(yyvsp[(3) - (3)].boolExpr));
    ;}
    break;

  case 37:
#line 612 "../src/C.y"
    {       if(verbose) printf(" OR ");
       (yyval.boolExpr) = new BPatch_boolExpr(BPatch_or, *(yyvsp[(1) - (3)].boolExpr), *(yyvsp[(3) - (3)].boolExpr));
    ;}
    break;

  case 38:
#line 622 "../src/C.y"
    {
      YYSTYPE::VariableSpec vSpec = {false,false,false,false,false,false,false,false,""};
      vSpec.isGlobal = true;
      (yyval.varSpec) = vSpec;   
   ;}
    break;

  case 39:
#line 628 "../src/C.y"
    {
      YYSTYPE::VariableSpec vSpec = {false,false,false,false,false,false,false,false,""};
      vSpec.isLocal = true;
      (yyval.varSpec) = vSpec;   
   ;}
    break;

  case 40:
#line 634 "../src/C.y"
    {
      YYSTYPE::VariableSpec vSpec = {false,false,false,false,false,false,false,false,""};
      vSpec.isParam = true;
      (yyval.varSpec) = vSpec;   
   ;}
    break;

  case 41:
#line 655 "../src/C.y"
    {
       (yyval.snippet) = snippetGen->findInstVariable(dynC_API::getMangledStub((yyvsp[(1) - (1)].sval), dynCSnippetName).c_str(), (yyvsp[(1) - (1)].sval));
       if((yyval.snippet) == NULL){
          (yyval.snippet) = new BPatch_nullExpr();
          char *errString = strdup(snippetGen->getError().c_str());
          yyerror(errString);
          free(errString);
          break;
       }
    ;}
    break;

  case 42:
#line 666 "../src/C.y"
    {
       (yyval.snippet) = snippetGen->findInstVariable(dynC_API::getMangledStub((yyvsp[(2) - (2)].sval), dynCSnippetName).c_str(), (yyvsp[(2) - (2)].sval));
       if((yyval.snippet) == NULL){
          (yyval.snippet) = new BPatch_nullExpr();
          char *errString = strdup(snippetGen->getError().c_str());
          yyerror(errString);
          free(errString);
          break;
       }
       (yyval.snippet) = (BPatch_snippet *)(new BPatch_arithExpr(BPatch_deref, *(yyval.snippet)));       
    ;}
    break;

  case 43:
#line 678 "../src/C.y"
    {
       (yyval.snippet) = snippetGen->findInstVariable(dynC_API::getMangledStub((yyvsp[(2) - (2)].sval), dynCSnippetName).c_str(), (yyvsp[(2) - (2)].sval));
       if((yyval.snippet) == NULL){
          (yyval.snippet) = new BPatch_nullExpr();
          char *errString = strdup(snippetGen->getError().c_str());
          yyerror(errString);
          free(errString);
          break;
       }
       (yyval.snippet) = (BPatch_snippet *)(new BPatch_arithExpr(BPatch_addr, *(yyval.snippet)));       
    ;}
    break;

  case 44:
#line 691 "../src/C.y"
    {
       (yyval.snippet) = snippetGen->findAppVariable((yyvsp[(3) - (3)].sval));
       if((yyval.snippet) == NULL){
          (yyval.snippet) = new BPatch_nullExpr();
          char *errString = strdup(snippetGen->getError().c_str());
          yyerrorNoTokNonUni(errString);
          free(errString);
          break;
       }
    ;}
    break;

  case 45:
#line 702 "../src/C.y"
    {
       (yyval.snippet) = snippetGen->findAppVariable((yyvsp[(4) - (4)].sval));
       if((yyval.snippet) == NULL){
          (yyval.snippet) = new BPatch_nullExpr();
          char *errString = strdup(snippetGen->getError().c_str());
          yyerrorNoTokNonUni(errString);
          free(errString);
          break;
       }
       (yyval.snippet) = (BPatch_snippet *)(new BPatch_arithExpr(BPatch_deref, *(yyval.snippet)));       
    ;}
    break;

  case 46:
#line 714 "../src/C.y"
    {
       (yyval.snippet) = snippetGen->findAppVariable((yyvsp[(4) - (4)].sval));
       if((yyval.snippet) == NULL){
          (yyval.snippet) = new BPatch_nullExpr();
          char *errString = strdup(snippetGen->getError().c_str());
          yyerrorNoTokNonUni(errString);
          free(errString);
          break;
       }
       (yyval.snippet) = (BPatch_snippet *)(new BPatch_arithExpr(BPatch_addr, *(yyval.snippet)));       
    ;}
    break;

  case 47:
#line 728 "../src/C.y"
    {
       //disallowed if there is no point specifier
       if(!(yyvsp[(1) - (3)].varSpec).isGlobal && snippetPoint == NULL){
          yyerrorNoTok("Local variables not allowed when snippet point is unspecified.");
          YYABORT;
          break;
       }
       if((yyvsp[(1) - (3)].varSpec).isParam){
          (yyval.snippet) = snippetGen->findParameter((yyvsp[(3) - (3)].sval));
       }else{
          (yyval.snippet) = snippetGen->findAppVariable((yyvsp[(3) - (3)].sval), (yyvsp[(1) - (3)].varSpec).isGlobal, (yyvsp[(1) - (3)].varSpec).isLocal);
       }
       if((yyval.snippet) == NULL){
          (yyval.snippet) = new BPatch_nullExpr();
          if((yyvsp[(1) - (3)].varSpec).isGlobal){
             char *errString = strdup(snippetGen->getError().c_str());
             yyerror(errString);
             free(errString);
             YYABORT;
          }else{
             char *errString = strdup(snippetGen->getError().c_str());
             yyerrorNonUni(errString);
             free(errString);
             YYABORT;
          }
          break;
       }
    ;}
    break;

  case 48:
#line 757 "../src/C.y"
    {
       if(snippetPoint == NULL){
          yyerrorNoTok("Local variables not allowed when snippet point is unspecified.");
          break;
       }       
       if((yyvsp[(2) - (4)].varSpec).isParam){
          (yyval.snippet) = snippetGen->findParameter((yyvsp[(4) - (4)].sval));
       }else{
          (yyval.snippet) = snippetGen->findAppVariable((yyvsp[(4) - (4)].sval), (yyvsp[(2) - (4)].varSpec).isGlobal, (yyvsp[(2) - (4)].varSpec).isLocal);
       }
       if((yyval.snippet) == NULL){
          (yyval.snippet) = new BPatch_nullExpr();
          if((yyvsp[(2) - (4)].varSpec).isGlobal){
             char *errString = strdup(snippetGen->getError().c_str());
             yyerror(errString);
             free(errString);
          }else{
             char *errString = strdup(snippetGen->getError().c_str());
             yyerrorNonUni(errString);
             free(errString);
          }
          break;
       }
       (yyval.snippet) = (BPatch_snippet *)(new BPatch_arithExpr(BPatch_deref, *(yyval.snippet)));       
    ;}
    break;

  case 49:
#line 784 "../src/C.y"
    {
       if(snippetPoint == NULL){
          yyerrorNoTok("Local variables not allowed when snippet point is unspecified.");
          break;
       }       
       if((yyvsp[(2) - (4)].varSpec).isParam){
          (yyval.snippet) = snippetGen->findParameter((yyvsp[(4) - (4)].sval));
       }else{
          (yyval.snippet) = snippetGen->findAppVariable((yyvsp[(4) - (4)].sval), (yyvsp[(2) - (4)].varSpec).isGlobal, (yyvsp[(2) - (4)].varSpec).isLocal);
       }
       if((yyval.snippet) == NULL){
          (yyval.snippet) = new BPatch_nullExpr();
          if((yyvsp[(2) - (4)].varSpec).isGlobal){
             char *errString = strdup(snippetGen->getError().c_str());
             yyerror(errString);
             free(errString);
          }else{
             char *errString = strdup(snippetGen->getError().c_str());
             yyerrorNonUni(errString);
             free(errString);
          }
          break;
       }
       (yyval.snippet) = (BPatch_snippet *)(new BPatch_arithExpr(BPatch_addr, *(yyval.snippet)));       
    ;}
    break;

  case 50:
#line 811 "../src/C.y"
    {
       //special case for indexed parameters
       if(snippetPoint == NULL){
          yyerrorNoTok("Local variables not allowed when snippet point is unspecified.");
          break;
       }       
       if(!(yyvsp[(1) - (3)].varSpec).isParam){
          yyerror("Numbered indexes for parameters only");
          (yyval.snippet) = new BPatch_nullExpr();
          break;
       }
       (yyval.snippet) = snippetGen->findParameter((yyvsp[(3) - (3)].ival));
       if((yyval.snippet) == NULL){
          (yyval.snippet) = new BPatch_nullExpr();
          char *errString = strdup(snippetGen->getError().c_str());
          yyerrorNoTokNonUni(errString);
          free(errString);
          break;
       }
    ;}
    break;

  case 51:
#line 832 "../src/C.y"
    {
       //special case for indexed parameters
       if(snippetPoint == NULL){
          yyerrorNoTok("Local variables not allowed when snippet point is unspecified.");
          break;
       }       
       if(!(yyvsp[(2) - (4)].varSpec).isParam){
          yyerror("Numbered indexes for parameters only");
          (yyval.snippet) = new BPatch_nullExpr();
          break;
       }
       (yyval.snippet) = snippetGen->findParameter((yyvsp[(4) - (4)].ival));
       if((yyval.snippet) == NULL){
          (yyval.snippet) = new BPatch_nullExpr();
          char *errString = strdup(snippetGen->getError().c_str());
          yyerrorNoTokNonUni(errString);
          free(errString);
          break;
       }
       (yyval.snippet) = (BPatch_snippet *)(new BPatch_arithExpr(BPatch_deref, *(yyval.snippet)));
    ;}
    break;

  case 52:
#line 854 "../src/C.y"
    {
       //special case for indexed parameters
       if(snippetPoint == NULL){
          yyerrorNoTok("Local variables not allowed when snippet point is unspecified.");
          break;
       }       

       if(!(yyvsp[(2) - (4)].varSpec).isParam){
          yyerror("Numbered indexes for parameters only");
          (yyval.snippet) = new BPatch_nullExpr();
          break;
       }
       (yyval.snippet) = snippetGen->findParameter((yyvsp[(4) - (4)].ival));
       if((yyval.snippet) == NULL){
          (yyval.snippet) = new BPatch_nullExpr();
          char *errString = strdup(snippetGen->getError().c_str());
          yyerrorNoTokNonUni(errString);
          free(errString);
          break;
       }
       (yyval.snippet) = (BPatch_snippet *)(new BPatch_arithExpr(BPatch_addr, *(yyval.snippet)));
    ;}
    break;

  case 53:
#line 878 "../src/C.y"
    {
       //array referance
       //check for integer in arith_expression
       (yyval.snippet) = snippetGen->generateArrayRef((yyvsp[(1) - (4)].snippet), (yyvsp[(3) - (4)].snippet));
       if((yyval.snippet) == NULL){
          (yyval.snippet) = new BPatch_nullExpr();
          char *errString = strdup(snippetGen->getError().c_str());
          yyerror(errString);
          free(errString);
          break;
       }
    ;}
    break;

  case 54:
#line 906 "../src/C.y"
    { 
      if(verbose) printf(" %d ", (yyvsp[(1) - (1)].ival));
      BPatch_snippet * c = new BPatch_constExpr((yyvsp[(1) - (1)].ival));
      (yyval.snippetStringPair) = new std::pair<BPatch_snippet *, char *>(c, "int");
   ;}
    break;

  case 55:
#line 912 "../src/C.y"
    { 
       if(verbose) printf(" %s ", (yyvsp[(1) - (1)].sval));
       BPatch_snippet * c = new BPatch_constExpr((yyvsp[(1) - (1)].sval));
       (yyval.snippetStringPair) = new std::pair<BPatch_snippet *, char *>(c, "char *");
    ;}
    break;

  case 56:
#line 921 "../src/C.y"
    {
        std::vector<std::pair<BPatch_snippet *, char *> > *cnlist = new std::vector<std::pair<BPatch_snippet *, char *> >();
        cnlist->push_back(*(yyvsp[(1) - (1)].snippetStringPair));
        (yyval.snippetStringListPair) = cnlist;
     ;}
    break;

  case 57:
#line 927 "../src/C.y"
    {
        (yyvsp[(1) - (3)].snippetStringListPair)->push_back(*(yyvsp[(3) - (3)].snippetStringPair));
        (yyval.snippetStringListPair) = (yyvsp[(1) - (3)].snippetStringListPair);
     ;}
    break;

  case 59:
#line 933 "../src/C.y"
    {(yyval.snippet) = (yyvsp[(1) - (1)].snippetStringPair)->first;}
    break;

  case 60:
#line 934 "../src/C.y"
    {(yyval.snippet) = new BPatch_nullExpr();;}
    break;

  case 61:
#line 936 "../src/C.y"
    {
       if(verbose) printf("dyninst`%s ", (yyvsp[(3) - (3)].sval));
       
       std::vector<BPatch_snippet *> argVect;
       //snippets w/ return vals
       if(strcmp((yyvsp[(3) - (3)].sval), "function_name") == 0){
          (yyval.snippet) = snippetGen->getContextInfo(SnippetGenerator::SG_FunctionName);
          if((yyval.snippet) == NULL){
             (yyval.snippet) = new BPatch_nullExpr();
             char *errString = strdup(snippetGen->getError().c_str());
             yyerror(errString);
             free(errString);
          }
          break;
       }
       if(strcmp((yyvsp[(3) - (3)].sval), "module_name") == 0){
          (yyval.snippet) = snippetGen->getContextInfo(SnippetGenerator::SG_ModuleName);
          if((yyval.snippet) == NULL){
             (yyval.snippet) = new BPatch_nullExpr();
             char *errString = strdup(snippetGen->getError().c_str());
             yyerror(errString);
             free(errString);
          }
          break;
       }
       if(strcmp((yyvsp[(3) - (3)].sval), "bytes_accessed") == 0){
          (yyval.snippet) = new BPatch_bytesAccessedExpr();
          break;
       }
       if(strcmp((yyvsp[(3) - (3)].sval), "effective_address") == 0){
          (yyval.snippet) = new BPatch_effectiveAddressExpr(); 
          break;
       }
       if(strcmp((yyvsp[(3) - (3)].sval), "original_address") == 0){
          (yyval.snippet) = new BPatch_originalAddressExpr();
          
          break;
       }
       if(strcmp((yyvsp[(3) - (3)].sval), "actual_address") == 0){
          (yyval.snippet) = new BPatch_actualAddressExpr();
          break;
       }
       if(strcmp((yyvsp[(3) - (3)].sval), "return_value") == 0){
          if(snippetGen->getPoint()->getPointType() != BPatch_exit){
             (yyval.snippet) = new BPatch_nullExpr();
             yyerrorNoTokNonUni("Return values only valid at function exit points");
             break;
          }
          (yyval.snippet) = new BPatch_retExpr();
          break;
       }
       if(strcmp((yyvsp[(3) - (3)].sval), "thread_index") == 0){
          (yyval.snippet) = new BPatch_threadIndexExpr();
          break;
       }
       if(strcmp((yyvsp[(3) - (3)].sval), "tid") == 0){
          (yyval.snippet) = snippetGen->getContextInfo(SnippetGenerator::SG_TID);
          if((yyval.snippet) == NULL){
             (yyval.snippet) = new BPatch_nullExpr();
             char *errString = strdup(snippetGen->getError().c_str());
             yyerror(errString);
             free(errString);
          }
          break;
       }
       if(strcmp((yyvsp[(3) - (3)].sval), "dynamic_target") == 0){
          (yyval.snippet) = new BPatch_dynamicTargetExpr();
          break;
       }
 
       yyerror("Syntax error: unrecognized dyninst call");
       (yyval.snippet) = new BPatch_nullExpr()
    ;}
    break;

  case 62:
#line 1010 "../src/C.y"
    {
       if(verbose) printf(" * ");
       (yyval.snippet) = new BPatch_arithExpr(BPatch_times, *(yyvsp[(1) - (3)].snippet), *(yyvsp[(3) - (3)].snippet));
       actionTaken = true;
    ;}
    break;

  case 63:
#line 1016 "../src/C.y"
    {
       (yyval.snippet) = (yyvsp[(1) - (1)].snippet);
       actionTaken = true;
    ;}
    break;

  case 64:
#line 1021 "../src/C.y"
    {
       if(verbose) printf(" = ");
	    (yyval.snippet) = new BPatch_arithExpr(BPatch_assign, *(yyvsp[(1) - (3)].snippet), *(yyvsp[(3) - (3)].snippet));
       actionTaken = true;
    ;}
    break;

  case 65:
#line 1027 "../src/C.y"
    {
       if(verbose) printf(" += ");
       (yyval.snippet) = new BPatch_arithExpr(BPatch_assign, *(yyvsp[(1) - (3)].snippet), BPatch_arithExpr(BPatch_plus, *(yyvsp[(1) - (3)].snippet), *(yyvsp[(3) - (3)].snippet)));
       actionTaken = true;
    ;}
    break;

  case 66:
#line 1033 "../src/C.y"
    {
       if(verbose) printf(" -= ");
       (yyval.snippet) = new BPatch_arithExpr(BPatch_assign, *(yyvsp[(1) - (3)].snippet), BPatch_arithExpr(BPatch_minus, *(yyvsp[(1) - (3)].snippet), *(yyvsp[(3) - (3)].snippet)));
       actionTaken = true;
    ;}
    break;

  case 67:
#line 1039 "../src/C.y"
    {
       if(verbose) printf(" *= ");
       (yyval.snippet) = new BPatch_arithExpr(BPatch_assign, *(yyvsp[(1) - (3)].snippet), BPatch_arithExpr(BPatch_times, *(yyvsp[(1) - (3)].snippet), *(yyvsp[(3) - (3)].snippet)));
       actionTaken = true;
    ;}
    break;

  case 68:
#line 1045 "../src/C.y"
    {
       if(verbose) printf(" /= ");
       (yyval.snippet) = new BPatch_arithExpr(BPatch_assign, *(yyvsp[(1) - (3)].snippet), BPatch_arithExpr(BPatch_divide, *(yyvsp[(1) - (3)].snippet), *(yyvsp[(3) - (3)].snippet)));
       actionTaken = true;
    ;}
    break;

  case 69:
#line 1051 "../src/C.y"
    {
       if(verbose) printf(" %%= ");
       (yyval.snippet) = new BPatch_arithExpr(BPatch_assign, *(yyvsp[(1) - (3)].snippet), BPatch_arithExpr(BPatch_minus, *(yyvsp[(1) - (3)].snippet), BPatch_arithExpr(BPatch_times, BPatch_arithExpr(BPatch_divide, *(yyvsp[(1) - (3)].snippet), *(yyvsp[(3) - (3)].snippet)), *(yyvsp[(3) - (3)].snippet))));
       actionTaken = true;
    ;}
    break;

  case 70:
#line 1057 "../src/C.y"
    {
       if(verbose) printf(" / ");
       (yyval.snippet) = new BPatch_arithExpr(BPatch_divide, *(yyvsp[(1) - (3)].snippet), *(yyvsp[(3) - (3)].snippet));
       actionTaken = true;
    ;}
    break;

  case 71:
#line 1063 "../src/C.y"
    {
       if(verbose) printf(" %% ");
       (yyval.snippet) = new BPatch_arithExpr(BPatch_minus, *(yyvsp[(1) - (3)].snippet), BPatch_arithExpr(BPatch_times, BPatch_arithExpr(BPatch_divide, *(yyvsp[(1) - (3)].snippet), *(yyvsp[(3) - (3)].snippet)), *(yyvsp[(3) - (3)].snippet)));
       actionTaken = true;
    ;}
    break;

  case 72:
#line 1069 "../src/C.y"
    {
       if(verbose) printf(" + ");
       (yyval.snippet) = new BPatch_arithExpr(BPatch_plus, *(yyvsp[(1) - (3)].snippet), *(yyvsp[(3) - (3)].snippet));
       actionTaken = true;
    ;}
    break;

  case 73:
#line 1075 "../src/C.y"
    {
       if(verbose) printf(" - ");
       (yyval.snippet) = new BPatch_arithExpr(BPatch_minus, *(yyvsp[(1) - (3)].snippet), *(yyvsp[(3) - (3)].snippet));
       actionTaken = true;
    ;}
    break;

  case 74:
#line 1081 "../src/C.y"
    {
       if(dynamic_cast<BPatch_nullExpr *>((yyvsp[(1) - (3)].snippet))){
          printf("Picked second\n");
          (yyval.snippet) = (yyvsp[(3) - (3)].snippet);
       }else{
          (yyval.snippet) = (yyvsp[(1) - (3)].snippet);
       }
    ;}
    break;

  case 75:
#line 1089 "../src/C.y"
    {(yyval.snippet) = (yyvsp[(2) - (3)].snippet);;}
    break;

  case 76:
#line 1091 "../src/C.y"
    {
       (yyval.snippet) = (yyvsp[(1) - (1)].snippet);
       actionTaken = true;
    ;}
    break;

  case 77:
#line 1100 "../src/C.y"
    {
       if(verbose) printf(" ++ ");
       BPatch_arithExpr addOne = BPatch_arithExpr(BPatch_assign, *(yyvsp[(1) - (2)].snippet), BPatch_arithExpr(BPatch_plus, *(yyvsp[(1) - (2)].snippet), BPatch_constExpr(1)));
       (yyval.snippet) = new BPatch_arithExpr(BPatch_seq, (BPatch_snippet &)addOne, BPatch_arithExpr(BPatch_minus, *(yyvsp[(1) - (2)].snippet), BPatch_constExpr(1)));
    ;}
    break;

  case 78:
#line 1106 "../src/C.y"
    {
       if(verbose) printf(" ++ ");
       BPatch_arithExpr addOne = BPatch_arithExpr(BPatch_assign, *(yyvsp[(2) - (2)].snippet), BPatch_arithExpr(BPatch_plus, *(yyvsp[(2) - (2)].snippet), BPatch_constExpr(1)));
       (yyval.snippet) = new BPatch_arithExpr(BPatch_seq, (BPatch_snippet &)addOne, *(yyvsp[(2) - (2)].snippet));
    ;}
    break;

  case 79:
#line 1112 "../src/C.y"
    {
       if(verbose) printf(" -- ");
       BPatch_arithExpr subOne = BPatch_arithExpr(BPatch_assign, *(yyvsp[(1) - (2)].snippet), BPatch_arithExpr(BPatch_minus, *(yyvsp[(1) - (2)].snippet), BPatch_constExpr(1)));
       (yyval.snippet) = new BPatch_arithExpr(BPatch_seq, (BPatch_snippet &)subOne, BPatch_arithExpr(BPatch_plus, *(yyvsp[(1) - (2)].snippet), BPatch_constExpr(1)));
    ;}
    break;

  case 80:
#line 1118 "../src/C.y"
    {
       if(verbose) printf(" -- ");
       BPatch_arithExpr subOne = BPatch_arithExpr(BPatch_assign, *(yyvsp[(2) - (2)].snippet), BPatch_arithExpr(BPatch_minus, *(yyvsp[(2) - (2)].snippet), BPatch_constExpr(1)));
       (yyval.snippet) = new BPatch_arithExpr(BPatch_seq, (BPatch_snippet &)subOne, *(yyvsp[(2) - (2)].snippet));
    ;}
    break;


/* Line 1267 of yacc.c.  */
#line 2888 "dynC.tab.c"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;
  *++yylsp = yyloc;

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

  yyerror_range[0] = yylloc;

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
		      yytoken, &yylval, &yylloc);
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

  yyerror_range[0] = yylsp[1-yylen];
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

      yyerror_range[0] = *yylsp;
      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp, yylsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;

  yyerror_range[1] = yylloc;
  /* Using YYLLOC is tempting, but would change the location of
     the look-ahead.  YYLOC is available though.  */
  YYLLOC_DEFAULT (yyloc, (yyerror_range - 1), 2);
  *++yylsp = yyloc;

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
		 yytoken, &yylval, &yylloc);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp, yylsp);
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


#line 1125 "../src/C.y"


#include <stdio.h>

std::stringstream * err;

void yyerror(char *s)
{
   fflush(stdout);
   if(strlen(s) != 0){
      std::stringstream err;
      err.str("");
      char ebase[256];
      getErrorBase(ebase, 256);
      err << ebase << " error: " << strdup(s) << " for token '" << lineStr << "'";
      if(universalErrors->find(err.str()) != universalErrors->end()){
         return;
      }
      printf("%s\n", err.str().c_str());
      universalErrors->insert(std::string(strdup(err.str().c_str())));
   }
}

void yyerrorNonUni(char *s){
   fflush(stdout);
   std::stringstream err;
   err.str("");
   char ebase[256];
   getErrorBase(ebase, 256);
   err << ebase << " error: " << s << " for token '" << lineStr << "'";
   printf("%s\n", err.str().c_str());
}

void getErrorBase(char *errbase, int length){
   char base[512] = "";
   sprintf(base, "%s:%d.%d:", dynCSnippetName, yylloc.first_line, yylloc.first_column);
   strncpy(errbase, base, (length > 512 ? 512 : length));
}

void yyerrorNoTok(char *s){
   fflush(stdout);
   std::stringstream err;
   err.str("");
   char ebase[256];
   getErrorBase(ebase, 256);
   err << ebase << " error: " << s;
   if(universalErrors->find(err.str()) != universalErrors->end()){
      return;
   }   
   printf("%s\n", err.str().c_str());
   universalErrors->insert(std::string(strdup(err.str().c_str())));
}


void yyerrorNoTokNonUni(char *s){
   char ebase[256];
   getErrorBase(ebase, 256);
   printf("%s error: %s\n", ebase, s);
}

void yywarn(char *s)
{
   std::stringstream err;
   err.str("");
   err << dynCSnippetName << ":" << yylloc.first_line << ":" << " warning: " << s;
   if(universalErrors->find(err.str()) != universalErrors->end()){
      return;
   }   
   printf("%s\n", err.str().c_str());
   universalErrors->insert(std::string(strdup(err.str().c_str())));
}

void writeMessage(char *s){
   fflush(mfile);
   fprintf(mfile, "At line %d: ", line_num);
   fprintf(mfile, "%s\n", s);   
}

void makeOneTimeStatement(BPatch_snippet &statement){
   std::stringstream mangledName;
   mangledName << "dynC_internal_" << dynCSnippetName << "_" << oneTimeCount++;
   BPatch_snippet * var = snippetGen->findOrCreateVariable(mangledName.str().c_str(), "int"); //will only allocate memory if nessessary
   BPatch_arithExpr *setFlag = new BPatch_arithExpr(BPatch_assign, *var, BPatch_constExpr(1));
   BPatch_arithExpr *pair = new BPatch_arithExpr(BPatch_seq, *setFlag, statement);
   BPatch_ifExpr *testFirst = new BPatch_ifExpr(BPatch_boolExpr(BPatch_eq, *var, BPatch_constExpr(0)), *pair);
   statement = *testFirst;    
}

void makeOneTimeStatementGbl(BPatch_snippet &statement){
   std::stringstream mangledName;
   mangledName << "dynC_internal_" << dynCSnippetName << "_" << oneTimeGblCount++;
   BPatch_snippet * var = snippetGen->findOrCreateVariable(mangledName.str().c_str(), "int"); //will only allocate memory if nessessary
   BPatch_arithExpr *setFlag = new BPatch_arithExpr(BPatch_assign, *var, BPatch_constExpr(1));
   BPatch_arithExpr *pair = new BPatch_arithExpr(BPatch_seq, *setFlag, statement);
   BPatch_ifExpr *testFirst = new BPatch_ifExpr(BPatch_boolExpr(BPatch_eq, *var, BPatch_constExpr(0)), *pair);
   statement = *testFirst;    
}

