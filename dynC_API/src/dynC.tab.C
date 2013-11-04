
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.
   
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
#define YYBISON_VERSION "2.4.1"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 1

/* Substitute the variable and function names.  */
#define yyparse         dynCparse
#define yylex           dynClex
#define yyerror         dynCerror
#define yylval          dynClval
#define yychar          dynCchar
#define yydebug         dynCdebug
#define yynerrs         dynCnerrs
#define yylloc          dynClloc

/* Copy the first part of user declarations.  */

/* Line 189 of yacc.c  */
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
   void yyerror(const char *s); 
   void yyerrorNonUni(const char *s);
   void yyerrorNoTok(const char *s);
   void yyerrorNoTokNonUni(const char *s);
   void yywarn(const char *s);
   int yyparse();
   void makeOneTimeStatement(BPatch_snippet &statement);
   void makeOneTimeStatementGbl(BPatch_snippet &statement);
   void getErrorBase(char *errbase, int length);
//   char *yytext;
//   char *dynCSnippetName;
}

extern std::string lineStr;

#define YYDEBUG 0 //set to 1 for debug mode

//name of current snippet for error reporting
const char *dynCSnippetName = "";

SnippetGenerator *snippetGen;
BPatch_point *snippetPoint = NULL;

std::set<std::string> *universalErrors = new std::set<std::string>();

int oneTimeCount = 0;
int oneTimeGblCount = 0;
int yylex();

BPatch_snippet *parse_result;

bool fatalError = false;
bool actionTaken = false;

extern bool interactive;
bool verbose = false;

extern int line_num;

std::vector<BPatch_snippet *> endSnippets;


/* Line 189 of yacc.c  */
#line 146 "dynC.tab.c"

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

/* Line 214 of yacc.c  */
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



/* Line 214 of yacc.c  */
#line 300 "dynC.tab.c"
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


/* Copy the second part of user declarations.  */


/* Line 264 of yacc.c  */
#line 325 "dynC.tab.c"

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
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
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
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
  YYLTYPE yyls_alloc;
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
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  45
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   516

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  104
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  17
/* YYNRULES -- Number of rules.  */
#define YYNRULES  75
/* YYNRULES -- Number of states.  */
#define YYNSTATES  147

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   341

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    82,     2,     2,     2,    72,    84,     2,
     100,   101,    70,    68,     2,    69,     2,    71,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     102,     2,   103,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    80,     2,    81,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    98,    73,    99,    83,     2,     2,     2,
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
      65,    66,    67,    74,    75,    76,    77,    78,    79,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97
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
     150,   152,   154,   158,   162,   166,   171,   174,   177,   179,
     181,   183,   187,   189,   191,   193,   197,   201,   205,   207,
     211,   215,   219,   223,   227,   231,   235,   239,   243,   247,
     251,   255,   257,   260,   263,   266
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
     105,     0,    -1,   108,    -1,   107,     4,    -1,   107,     4,
      85,   119,    -1,   107,     4,    80,     8,    81,    -1,   107,
       4,    80,    81,    85,    98,   118,    99,    -1,   107,     4,
      80,     8,    81,    85,    98,   118,    99,    -1,     7,    -1,
      27,   107,    -1,   109,    -1,   108,   109,    -1,    94,   108,
      93,    -1,   108,    94,   108,    93,    -1,     1,    -1,     9,
      -1,   106,    75,    -1,   119,    75,    -1,    42,   100,   114,
     101,   110,    -1,    42,   100,   114,   101,   110,    96,   110,
      -1,   109,    -1,    98,   108,    99,    -1,    47,    79,     4,
     100,   112,   101,    -1,    46,    79,     4,   100,   112,   101,
      -1,    -1,   119,    -1,   112,    57,   119,    -1,    12,    -1,
      13,    -1,   113,    -1,   119,   102,   119,    -1,   119,   103,
     119,    -1,   119,    65,   119,    -1,   119,    67,   119,    -1,
     119,    66,   119,    -1,   119,    64,   119,    -1,   114,    63,
     114,    -1,   114,    62,   114,    -1,    45,    -1,    43,    -1,
      44,    -1,     4,    -1,    46,    79,     4,    -1,   115,    79,
       4,    -1,   115,    79,     8,    -1,   116,    80,   119,    81,
      -1,    70,   116,    -1,    84,   116,    -1,     8,    -1,     6,
      -1,   117,    -1,   118,    57,   117,    -1,   116,    -1,   117,
      -1,    55,    -1,    49,    79,     4,    -1,    47,    79,     4,
      -1,   119,    70,   119,    -1,   111,    -1,   116,    85,   119,
      -1,   116,    87,   119,    -1,   116,    86,   119,    -1,   116,
      90,   119,    -1,   116,    89,   119,    -1,   116,    88,   119,
      -1,   119,    71,   119,    -1,   119,    72,   119,    -1,   119,
      68,   119,    -1,   119,    69,   119,    -1,   119,    73,   119,
      -1,   100,   119,   101,    -1,   120,    -1,   116,    15,    -1,
      15,   116,    -1,   116,    16,    -1,    16,   116,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   164,   164,   187,   207,   225,   256,   289,   337,   342,
     355,   361,   366,   374,   384,   389,   395,   400,   411,   419,
     430,   432,   439,   454,   469,   474,   480,   489,   493,   500,
     501,   506,   511,   516,   521,   526,   531,   536,   544,   550,
     556,   565,   574,   584,   610,   630,   641,   645,   652,   658,
     667,   673,   679,   680,   681,   682,   691,   759,   765,   770,
     776,   782,   788,   794,   800,   806,   812,   818,   824,   830,
     839,   840,   849,   855,   861,   867
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
  "FUNC", "DYNINST", "INST", "REGISTER", "NEWLINE", "CASE", "DEFAULT",
  "SWITCH", "RETURN", "NILL", "EOF_TOK", "COMMA", "AMPERSAND", "ASTERISK",
  "DOT", "NOT", "OR", "AND", "NOT_EQ", "EQ", "GREATER_EQ", "LESS_EQ",
  "'+'", "'-'", "'*'", "'/'", "'%'", "'|'", "COLON", "SEMI", "END_BLOCK",
  "START_BLOCK", "DOLLAR", "BACKTICK", "'['", "']'", "'!'", "'~'", "'&'",
  "ASSIGN", "SUB_ASSIGN", "ADD_ASSIGN", "MOD_ASSIGN", "DIV_ASSIGN",
  "MUL_ASSIGN", "OR_OP", "AND_OP", "NCLOSE", "NOPEN", "LOWER_THAN_ELSE",
  "ELSE", "LOWER_THAN_DEREF", "'{'", "'}'", "'('", "')'", "'<'", "'>'",
  "$accept", "start", "var_declaration", "var_decl_modifiers",
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
     315,   316,   317,   318,   319,   320,   321,   322,    43,    45,
      42,    47,    37,   124,   323,   324,   325,   326,   327,   328,
      91,    93,    33,   126,    38,   329,   330,   331,   332,   333,
     334,   335,   336,   337,   338,   339,   340,   341,   123,   125,
      40,    41,    60,    62
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,   104,   105,   106,   106,   106,   106,   106,   107,   107,
     108,   108,   108,   108,   109,   109,   109,   109,   109,   109,
     110,   110,   111,   111,   112,   112,   112,   113,   113,   114,
     114,   114,   114,   114,   114,   114,   114,   114,   115,   115,
     115,   116,   116,   116,   116,   116,   116,   116,   117,   117,
     118,   118,   119,   119,   119,   119,   119,   119,   119,   119,
     119,   119,   119,   119,   119,   119,   119,   119,   119,   119,
     119,   119,   120,   120,   120,   120
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     2,     4,     5,     8,     9,     1,     2,
       1,     2,     3,     4,     1,     1,     2,     2,     5,     7,
       1,     3,     6,     6,     0,     1,     3,     1,     1,     1,
       3,     3,     3,     3,     3,     3,     3,     3,     1,     1,
       1,     1,     3,     3,     3,     4,     2,     2,     1,     1,
       1,     3,     1,     1,     1,     3,     3,     3,     1,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     1,     2,     2,     2,     2
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,    14,    41,    49,     8,    48,    15,     0,     0,     0,
       0,    39,    40,    38,     0,     0,     0,    54,     0,     0,
       0,     0,     0,     0,     0,     0,    10,    58,     0,    52,
      53,     0,    71,     0,    73,    75,     9,     0,     0,     0,
       0,    46,    47,     0,     0,     1,    16,     3,     0,    11,
       0,    72,    74,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    17,     0,    27,    28,
      29,     0,     0,    42,    56,    55,    12,    70,     0,     0,
       0,    43,    44,     0,    59,    61,    60,    64,    63,    62,
      67,    68,    57,    65,    66,    69,    42,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    24,    24,     0,     0,
       4,    13,    45,    37,    36,     0,    20,    18,    35,    32,
      34,    33,    30,    31,     0,    25,     0,     5,     0,     0,
       0,     0,    23,    22,     0,     0,    21,    19,    26,     0,
      50,     0,     0,     0,     6,     7,    51
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    22,    23,    24,    25,    26,   117,    27,   124,    70,
      71,    28,    29,    30,   141,    31,    32
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -126
static const yytype_int16 yypact[] =
{
     287,  -126,  -126,  -126,  -126,  -126,  -126,    23,    23,    -1,
     -85,  -126,  -126,  -126,   -62,   -51,   -48,  -126,    23,    23,
     287,   416,    25,    -4,    53,    45,  -126,  -126,    -6,   112,
    -126,   118,  -126,    19,    21,    21,  -126,   364,    95,    98,
      99,    21,  -126,   110,    62,  -126,  -126,   -56,   287,  -126,
      66,  -126,  -126,   416,   416,   416,   416,   416,   416,   416,
     416,   416,   416,   416,   416,   416,  -126,   100,  -126,  -126,
    -126,   -42,   385,     8,     9,  -126,  -126,  -126,    -3,   416,
     169,  -126,  -126,   177,  -126,  -126,  -126,  -126,  -126,  -126,
      -8,    -8,    39,    39,    39,  -126,  -126,   364,   364,   346,
     416,   416,   416,   416,   416,   416,   416,   416,    32,    35,
      78,  -126,  -126,    58,  -126,   287,  -126,    26,    78,    78,
      78,    78,    78,    78,   -54,    78,   -53,    38,    40,   228,
     346,   416,  -126,  -126,    42,     1,  -126,  -126,    78,     1,
    -126,   -49,   -44,     1,  -126,  -126,  -126
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
    -126,  -126,  -126,   115,   -18,   -24,     6,  -126,    34,  -126,
      -2,  -126,     4,  -125,     3,   -21,  -126
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -3
static const yytype_int16 yytable[] =
{
      44,    49,    43,   131,   131,   108,     4,     3,   143,     5,
     140,    34,    35,   143,   140,    37,    72,    38,   146,    49,
      97,    98,    41,    42,    78,    45,     9,     2,    39,    79,
      80,    40,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    -2,     1,   132,   133,     2,
     144,     3,     4,     5,     6,   145,    49,    47,   110,    99,
       7,     8,    62,    63,    64,    65,    11,    12,    13,    33,
      81,    46,     9,    50,    82,   116,    72,    72,   109,   118,
     119,   120,   121,   122,   123,   125,   125,    10,    11,    12,
      13,    14,    15,    18,    16,   113,   114,   129,    67,    73,
      17,    53,    74,    75,    96,    49,   116,    19,   106,   107,
     138,     1,    65,   127,     2,    18,     3,     4,     5,     6,
     128,    98,   130,   134,    36,     7,     8,    51,    52,    19,
      60,    61,    62,    63,    64,    65,   137,     9,   135,    48,
     139,   126,   142,     0,     0,    21,    60,    61,    62,    63,
      64,    65,    10,    11,    12,    13,    14,    15,     0,    16,
       0,     0,     0,    77,     0,    17,     0,     0,     0,     0,
       1,     0,     0,     2,     0,     3,     4,     5,     6,     0,
      18,     0,     0,     0,     7,     8,    60,    61,    62,    63,
      64,    65,    53,    66,    19,     0,     9,    54,    55,    56,
      57,    58,    59,    76,    48,     0,     0,     0,     0,     0,
      21,    10,    11,    12,    13,    14,    15,     0,    16,     0,
       0,     0,     0,     0,    17,     0,     0,     0,     0,     1,
       0,     0,     2,     0,     3,     4,     5,     6,     0,    18,
       0,     0,     0,     7,     8,    60,    61,    62,    63,    64,
      65,     0,     0,    19,     0,     9,     0,     0,   112,     0,
       0,     0,   111,    48,     0,     0,     0,     0,     0,    21,
      10,    11,    12,    13,    14,    15,     0,    16,     0,     0,
       0,     0,     0,    17,     0,     0,     0,     0,     1,     0,
       0,     2,     0,     3,     4,     5,     6,     0,    18,     0,
       0,     0,     7,     8,     0,     0,     0,     0,     0,     0,
       0,     0,    19,     0,     9,     0,     0,     0,     0,     0,
       0,     0,    48,     0,     0,     0,     0,   136,    21,    10,
      11,    12,    13,    14,    15,     0,    16,     0,     0,     0,
       0,     0,    17,     0,     0,     0,     0,     1,     0,     0,
       2,     0,     3,     4,     5,     6,     0,    18,     0,     0,
       0,     7,     8,     0,     0,     0,     0,     0,     2,     0,
       3,    19,     5,     9,     0,     0,    68,    69,     0,     7,
       8,    20,     0,     0,     0,     0,     0,    21,    10,    11,
      12,    13,    14,    15,     0,    16,     0,     0,     0,     0,
       0,    17,     0,     0,     0,     0,     0,    11,    12,    13,
      14,    15,     0,    16,     0,     0,    18,     0,     0,    17,
       2,     0,     3,     0,     5,     0,     0,     0,     0,     0,
      19,     7,     8,     0,    18,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   115,     0,    21,     0,    19,   100,
     101,   102,   103,    60,    61,    62,    63,    64,    65,    11,
      12,    13,    14,    15,    21,    16,     0,     0,     0,     0,
       0,    17,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    18,   104,   105,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      19,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    21
};

static const yytype_int16 yycheck[] =
{
      21,    25,    20,    57,    57,     8,     7,     6,    57,     8,
     135,     7,     8,    57,   139,   100,    37,    79,   143,    43,
      62,    63,    18,    19,    80,     0,    27,     4,    79,    85,
      48,    79,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,     0,     1,   101,   101,     4,
      99,     6,     7,     8,     9,    99,    80,     4,    79,   101,
      15,    16,    70,    71,    72,    73,    43,    44,    45,    46,
       4,    75,    27,    79,     8,    99,    97,    98,    81,   100,
     101,   102,   103,   104,   105,   106,   107,    42,    43,    44,
      45,    46,    47,    70,    49,    97,    98,   115,    79,     4,
      55,    80,     4,     4,     4,   129,   130,    84,   100,   100,
     131,     1,    73,    81,     4,    70,     6,     7,     8,     9,
      85,    63,    96,    85,     9,    15,    16,    15,    16,    84,
      68,    69,    70,    71,    72,    73,   130,    27,    98,    94,
      98,   107,   139,    -1,    -1,   100,    68,    69,    70,    71,
      72,    73,    42,    43,    44,    45,    46,    47,    -1,    49,
      -1,    -1,    -1,   101,    -1,    55,    -1,    -1,    -1,    -1,
       1,    -1,    -1,     4,    -1,     6,     7,     8,     9,    -1,
      70,    -1,    -1,    -1,    15,    16,    68,    69,    70,    71,
      72,    73,    80,    75,    84,    -1,    27,    85,    86,    87,
      88,    89,    90,    93,    94,    -1,    -1,    -1,    -1,    -1,
     100,    42,    43,    44,    45,    46,    47,    -1,    49,    -1,
      -1,    -1,    -1,    -1,    55,    -1,    -1,    -1,    -1,     1,
      -1,    -1,     4,    -1,     6,     7,     8,     9,    -1,    70,
      -1,    -1,    -1,    15,    16,    68,    69,    70,    71,    72,
      73,    -1,    -1,    84,    -1,    27,    -1,    -1,    81,    -1,
      -1,    -1,    93,    94,    -1,    -1,    -1,    -1,    -1,   100,
      42,    43,    44,    45,    46,    47,    -1,    49,    -1,    -1,
      -1,    -1,    -1,    55,    -1,    -1,    -1,    -1,     1,    -1,
      -1,     4,    -1,     6,     7,     8,     9,    -1,    70,    -1,
      -1,    -1,    15,    16,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    84,    -1,    27,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    94,    -1,    -1,    -1,    -1,    99,   100,    42,
      43,    44,    45,    46,    47,    -1,    49,    -1,    -1,    -1,
      -1,    -1,    55,    -1,    -1,    -1,    -1,     1,    -1,    -1,
       4,    -1,     6,     7,     8,     9,    -1,    70,    -1,    -1,
      -1,    15,    16,    -1,    -1,    -1,    -1,    -1,     4,    -1,
       6,    84,     8,    27,    -1,    -1,    12,    13,    -1,    15,
      16,    94,    -1,    -1,    -1,    -1,    -1,   100,    42,    43,
      44,    45,    46,    47,    -1,    49,    -1,    -1,    -1,    -1,
      -1,    55,    -1,    -1,    -1,    -1,    -1,    43,    44,    45,
      46,    47,    -1,    49,    -1,    -1,    70,    -1,    -1,    55,
       4,    -1,     6,    -1,     8,    -1,    -1,    -1,    -1,    -1,
      84,    15,    16,    -1,    70,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    98,    -1,   100,    -1,    84,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    43,
      44,    45,    46,    47,   100,    49,    -1,    -1,    -1,    -1,
      -1,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    70,   102,   103,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      84,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   100
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,     4,     6,     7,     8,     9,    15,    16,    27,
      42,    43,    44,    45,    46,    47,    49,    55,    70,    84,
      94,   100,   105,   106,   107,   108,   109,   111,   115,   116,
     117,   119,   120,    46,   116,   116,   107,   100,    79,    79,
      79,   116,   116,   108,   119,     0,    75,     4,    94,   109,
      79,    15,    16,    80,    85,    86,    87,    88,    89,    90,
      68,    69,    70,    71,    72,    73,    75,    79,    12,    13,
     113,   114,   119,     4,     4,     4,    93,   101,    80,    85,
     108,     4,     8,   119,   119,   119,   119,   119,   119,   119,
     119,   119,   119,   119,   119,   119,     4,    62,    63,   101,
      64,    65,    66,    67,   102,   103,   100,   100,     8,    81,
     119,    93,    81,   114,   114,    98,   109,   110,   119,   119,
     119,   119,   119,   119,   112,   119,   112,    81,    85,   108,
      96,    57,   101,   101,    85,    98,    99,   110,   119,    98,
     117,   118,   118,    57,    99,    99,   117
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
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
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
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       , &(yylsp[(yyi + 1) - (yynrhs)])		       );
      YYFPRINTF (stderr, "\n");
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


/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Location data for the lookahead symbol.  */
YYLTYPE yylloc;

/* Number of syntax errors so far.  */
int yynerrs;



/*-------------------------.
| yyparse or yypush_parse.  |
`-------------------------*/

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
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.
       `yyls': related to locations.

       Refer to the stacks thru separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    /* The location stack.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls;
    YYLTYPE *yylsp;

    /* The locations where the error started and ended.  */
    YYLTYPE yyerror_range[2];

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yyls = yylsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

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
  yylloc.first_column = yylloc.last_column = 1;
#endif

/* User initialization code.  */

/* Line 1242 of yacc.c  */
#line 96 "../src/C.y"
{
#ifdef YYDEBUG
#if YYDEBUG == 1
   yydebug = 1;
#endif
#endif
 }

/* Line 1242 of yacc.c  */
#line 1603 "dynC.tab.c"

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
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
	YYSTACK_RELOCATE (yyls_alloc, yyls);
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

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
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

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
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

/* Line 1455 of yacc.c  */
#line 165 "../src/C.y"
    { 
       oneTimeCount = 0;
       oneTimeGblCount = 0;
       if(fatalError){
          parse_result = NULL;
       }else{
          (yyvsp[(1) - (1)].snippetList)->insert((yyvsp[(1) - (1)].snippetList)->end(), endSnippets.begin(), endSnippets.end());
          parse_result = new BPatch_sequence(*(yyvsp[(1) - (1)].snippetList)); 
       }

       fatalError = false;
       delete (yyvsp[(1) - (1)].snippetList);

       endSnippets.clear();
       if(verbose) {
          printf("\n");
          fflush(stdout);
       }
       YYACCEPT;
    ;}
    break;

  case 3:

/* Line 1455 of yacc.c  */
#line 188 "../src/C.y"
    {
       std::string mangledName;
       mangledName = dynC_API::mangle((yyvsp[(2) - (2)].sval), dynCSnippetName, (yyvsp[(1) - (2)].varSpec).type);
       if(verbose) printf("name : %s\n", mangledName.c_str());
       (yyval.snippet) = snippetGen->findOrCreateVariable(mangledName.c_str(), (yyvsp[(1) - (2)].varSpec).type);
       if((yyval.snippet) == NULL){
          (yyval.snippet) = new BPatch_nullExpr();
          yyerror(snippetGen->getError().c_str());
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

/* Line 1455 of yacc.c  */
#line 208 "../src/C.y"
    {   
      
       std::string mangledName = dynC_API::mangle((yyvsp[(2) - (4)].sval), dynCSnippetName, (yyvsp[(1) - (4)].varSpec).type);
       if(verbose) printf("name : %s\n", mangledName.c_str());
       BPatch_snippet *alloc = snippetGen->findOrCreateVariable(mangledName.c_str(), (yyvsp[(1) - (4)].varSpec).type);
       if(alloc == NULL){
          (yyval.snippet) = new BPatch_nullExpr();
          yyerror(snippetGen->getError().c_str());
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

/* Line 1455 of yacc.c  */
#line 226 "../src/C.y"
    {
       //IDENTIFIER leaks, but how to fix b/c use of $0?
       if((yyvsp[(4) - (5)].ival) < 0){
          std::stringstream errMessage;
          errMessage << "Invalid array size: " << (yyvsp[(4) - (5)].ival);
          yyerrorNoTok(errMessage.str().c_str());
          (yyval.snippet) = new BPatch_nullExpr();
          break;
       }
      
       std::stringstream type;
       type << (yyvsp[(1) - (5)].varSpec).type << "[" << (yyvsp[(4) - (5)].ival) << "]";
       std::string mangledName = dynC_API::mangle((yyvsp[(2) - (5)].sval), dynCSnippetName, type.str().c_str());
       (yyval.snippet) = snippetGen->findOrCreateArray(mangledName.c_str(), (yyvsp[(1) - (5)].varSpec).type, (yyvsp[(4) - (5)].ival));
       if((yyval.snippet) == NULL){
          (yyval.snippet) = new BPatch_nullExpr();
          yyerror(snippetGen->getError().c_str());
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

/* Line 1455 of yacc.c  */
#line 257 "../src/C.y"
    {
       std::stringstream type;
       type << (yyvsp[(1) - (8)].varSpec).type << "[" << (yyvsp[(7) - (8)].snippetStringListPair)->size() << "]";
       std::string mangledName = dynC_API::mangle((yyvsp[(2) - (8)].sval), dynCSnippetName, type.str().c_str());

       (yyval.snippet) = snippetGen->findOrCreateArray(mangledName.c_str(), (yyvsp[(1) - (8)].varSpec).type, (yyvsp[(7) - (8)].snippetStringListPair)->size()); //will only allocate memory if nessessary
       if((yyval.snippet) == NULL){
          (yyval.snippet) = new BPatch_nullExpr();
          yyerror(snippetGen->getError().c_str());
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
             yyerrorNoTok(errMessage.c_str());
             (yyval.snippet) = new BPatch_nullExpr();
             break;
          }
          BPatch_snippet *assign = new BPatch_arithExpr(BPatch_assign, BPatch_arithExpr(BPatch_ref, *(yyval.snippet), BPatch_constExpr(n)), *(*(yyvsp[(7) - (8)].snippetStringListPair))[n].first);      
          
          if((yyvsp[(1) - (8)].varSpec).isStatic || (yyvsp[(1) - (8)].varSpec).isGlobal){
              makeOneTimeStatement(*assign);
              }
          assignVect->push_back(assign);
       }
       (yyval.snippet) = new BPatch_sequence(*assignVect);
    ;}
    break;

  case 7:

/* Line 1455 of yacc.c  */
#line 290 "../src/C.y"
    {
       std::vector<BPatch_snippet *> argVect;
       std::stringstream type;
       if((yyvsp[(4) - (9)].ival) < 0){
          std::stringstream errMessage;
          errMessage << "Invalid array size: " << (yyvsp[(4) - (9)].ival);
          yyerrorNoTok(errMessage.str().c_str());
          (yyval.snippet) = new BPatch_nullExpr();
          break;
       }
       type << (yyvsp[(1) - (9)].varSpec).type << "[" << (yyvsp[(4) - (9)].ival) << "]";
       std::string mangledName = dynC_API::mangle((yyvsp[(2) - (9)].sval), dynCSnippetName, type.str().c_str());

       (yyval.snippet) = snippetGen->findOrCreateArray(mangledName.c_str(), (yyvsp[(1) - (9)].varSpec).type, (yyvsp[(4) - (9)].ival)); //will only allocate memory if nessessary
       if((yyval.snippet) == NULL){
          (yyval.snippet) = new BPatch_nullExpr();
          yyerror(snippetGen->getError().c_str());
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
             yyerrorNoTok(errMessage.c_str());
             (yyval.snippet) = new BPatch_nullExpr();
             break;
          }
          BPatch_snippet *assign = new BPatch_arithExpr(BPatch_assign, BPatch_arithExpr(BPatch_ref, *(yyval.snippet), BPatch_constExpr(n)), *(*(yyvsp[(8) - (9)].snippetStringListPair))[n].first);      
          if((yyvsp[(1) - (9)].varSpec).isStatic || (yyvsp[(1) - (9)].varSpec).isGlobal){
             makeOneTimeStatement(*assign);
          }
          assignVect->push_back(assign);
       
       }
       (yyval.snippet) = new BPatch_sequence(*assignVect);
    ;}
    break;

  case 8:

/* Line 1455 of yacc.c  */
#line 338 "../src/C.y"
    {
      YYSTYPE::VariableSpec rSpec = {false,false,false,false,false,false,false,false,(yyvsp[(1) - (1)].sval)};
      (yyval.varSpec) = rSpec;
   ;}
    break;

  case 9:

/* Line 1455 of yacc.c  */
#line 343 "../src/C.y"
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

/* Line 1455 of yacc.c  */
#line 356 "../src/C.y"
    { 
       if(verbose) printf("\n");
       (yyval.snippetList) = new BPatch_Vector<BPatch_snippet *>; 
       (yyval.snippetList)->push_back((yyvsp[(1) - (1)].snippet));
    ;}
    break;

  case 11:

/* Line 1455 of yacc.c  */
#line 362 "../src/C.y"
    {
       (yyvsp[(1) - (2)].snippetList)->push_back((yyvsp[(2) - (2)].snippet));
       (yyval.snippetList) = (yyvsp[(1) - (2)].snippetList);
    ;}
    break;

  case 12:

/* Line 1455 of yacc.c  */
#line 367 "../src/C.y"
    {
       BPatch_sequence *seq = new BPatch_sequence(*(yyvsp[(2) - (3)].snippetList));
       makeOneTimeStatementGbl(*seq);
       std::vector<BPatch_snippet *> *retVect = new std::vector<BPatch_snippet *>;
       retVect->push_back(seq);
       (yyval.snippetList) = retVect;       
    ;}
    break;

  case 13:

/* Line 1455 of yacc.c  */
#line 375 "../src/C.y"
    {
       BPatch_sequence seq = BPatch_sequence(*(yyvsp[(3) - (4)].snippetList));
       makeOneTimeStatementGbl(seq);
       (yyvsp[(1) - (4)].snippetList)->push_back(&seq);
       (yyval.snippetList) = (yyvsp[(1) - (4)].snippetList);
    ;}
    break;

  case 14:

/* Line 1455 of yacc.c  */
#line 385 "../src/C.y"
    {
       (yyval.snippet) = new BPatch_nullExpr();
       actionTaken = false;
    ;}
    break;

  case 15:

/* Line 1455 of yacc.c  */
#line 390 "../src/C.y"
    {
       yyerrorNoTok((yyvsp[(1) - (1)].context));
       (yyval.snippet) = new BPatch_nullExpr();
       actionTaken = false;
    ;}
    break;

  case 16:

/* Line 1455 of yacc.c  */
#line 396 "../src/C.y"
    {
       (yyval.snippet) = (yyvsp[(1) - (2)].snippet);
    ;}
    break;

  case 17:

/* Line 1455 of yacc.c  */
#line 401 "../src/C.y"
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

/* Line 1455 of yacc.c  */
#line 412 "../src/C.y"
    {
       if(verbose) printf(" if () ");
       (yyval.snippet) = new BPatch_ifExpr(*(yyvsp[(3) - (5)].boolExpr), *(yyvsp[(5) - (5)].snippet));
       delete (yyvsp[(3) - (5)].boolExpr);
       delete (yyvsp[(5) - (5)].snippet);
    ;}
    break;

  case 19:

/* Line 1455 of yacc.c  */
#line 420 "../src/C.y"
    {
       if(verbose) printf(" if () else ");
       (yyval.snippet) = new BPatch_ifExpr(*(yyvsp[(3) - (7)].boolExpr), *(yyvsp[(5) - (7)].snippet), *(yyvsp[(7) - (7)].snippet));
       delete (yyvsp[(3) - (7)].boolExpr);
       delete (yyvsp[(5) - (7)].snippet);
       delete (yyvsp[(7) - (7)].snippet);
    ;}
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 433 "../src/C.y"
    {
        (yyval.snippet) = new BPatch_sequence(*(yyvsp[(2) - (3)].snippetList));
        delete (yyvsp[(2) - (3)].snippetList);
     ;}
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 440 "../src/C.y"
    {
       if(strcmp((yyvsp[(3) - (6)].sval), "break") == 0){
          if(verbose) printf("break_ ()");
          (yyval.snippet) = new BPatch_breakPointExpr();
       }else{
          char *errString = (char *)calloc(strlen((yyvsp[(3) - (6)].sval)), sizeof(char));
          sprintf(errString, "%s not found!\n", (yyvsp[(3) - (6)].sval));
          yyerror(errString);
          (yyval.snippet) = new BPatch_nullExpr();
          free(errString);
       }
       delete (yyvsp[(3) - (6)].sval);
    ;}
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 456 "../src/C.y"
    { 
       BPatch_function *func = snippetGen->findFunction((yyvsp[(3) - (6)].sval), *(yyvsp[(5) - (6)].snippetList));
       if(func == NULL){
          (yyval.snippet) = new BPatch_nullExpr();
          yyerrorNoTok(snippetGen->getError().c_str());
          break;
       }
       (yyval.snippet) = new BPatch_funcCallExpr(*func, *(yyvsp[(5) - (6)].snippetList));
    ;}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 469 "../src/C.y"
    {
       //No parameters, return an empty vector
       (yyval.snippetList) = new BPatch_Vector<BPatch_snippet *>;
    ;}
    break;

  case 25:

/* Line 1455 of yacc.c  */
#line 475 "../src/C.y"
    { 
       (yyval.snippetList) = new BPatch_Vector<BPatch_snippet *>; 
       (yyval.snippetList)->push_back((yyvsp[(1) - (1)].snippet));
    ;}
    break;

  case 26:

/* Line 1455 of yacc.c  */
#line 481 "../src/C.y"
    { 
       if(verbose) printf(" , ");
       (yyvsp[(1) - (3)].snippetList)->push_back((yyvsp[(3) - (3)].snippet)); 
       (yyval.snippetList) = (yyvsp[(1) - (3)].snippetList);
    ;}
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 490 "../src/C.y"
    { 
       (yyval.boolExpr) = new BPatch_boolExpr(BPatch_eq, BPatch_constExpr(0), BPatch_constExpr(0));
    ;}
    break;

  case 28:

/* Line 1455 of yacc.c  */
#line 494 "../src/C.y"
    {
       (yyval.boolExpr) = new BPatch_boolExpr(BPatch_ne, BPatch_constExpr(0), BPatch_constExpr(0));
    ;}
    break;

  case 30:

/* Line 1455 of yacc.c  */
#line 502 "../src/C.y"
    {
       if(verbose) printf(" < ");
       (yyval.boolExpr) = new BPatch_boolExpr(BPatch_lt, *(yyvsp[(1) - (3)].snippet), *(yyvsp[(3) - (3)].snippet));
    ;}
    break;

  case 31:

/* Line 1455 of yacc.c  */
#line 507 "../src/C.y"
    {
       if(verbose) printf(" > ");
       (yyval.boolExpr) = new BPatch_boolExpr(BPatch_gt, *(yyvsp[(1) - (3)].snippet), *(yyvsp[(3) - (3)].snippet));
    ;}
    break;

  case 32:

/* Line 1455 of yacc.c  */
#line 512 "../src/C.y"
    {
       if(verbose) printf(" == ");
       (yyval.boolExpr) = new BPatch_boolExpr(BPatch_eq, *(yyvsp[(1) - (3)].snippet), *(yyvsp[(3) - (3)].snippet));
    ;}
    break;

  case 33:

/* Line 1455 of yacc.c  */
#line 517 "../src/C.y"
    {
       if(verbose) printf(" <= ");
       (yyval.boolExpr) = new BPatch_boolExpr(BPatch_le, *(yyvsp[(1) - (3)].snippet), *(yyvsp[(3) - (3)].snippet));
    ;}
    break;

  case 34:

/* Line 1455 of yacc.c  */
#line 522 "../src/C.y"
    {
       if(verbose) printf(" >= ");
       (yyval.boolExpr) = new BPatch_boolExpr(BPatch_ge, *(yyvsp[(1) - (3)].snippet), *(yyvsp[(3) - (3)].snippet));
    ;}
    break;

  case 35:

/* Line 1455 of yacc.c  */
#line 527 "../src/C.y"
    {
       if(verbose) printf(" != ");
       (yyval.boolExpr) = new BPatch_boolExpr(BPatch_ne, *(yyvsp[(1) - (3)].snippet), *(yyvsp[(3) - (3)].snippet));
    ;}
    break;

  case 36:

/* Line 1455 of yacc.c  */
#line 532 "../src/C.y"
    {
       if(verbose) printf(" AND ");
       (yyval.boolExpr) = new BPatch_boolExpr(BPatch_and, *(yyvsp[(1) - (3)].boolExpr), *(yyvsp[(3) - (3)].boolExpr));
    ;}
    break;

  case 37:

/* Line 1455 of yacc.c  */
#line 537 "../src/C.y"
    {       if(verbose) printf(" OR ");
       (yyval.boolExpr) = new BPatch_boolExpr(BPatch_or, *(yyvsp[(1) - (3)].boolExpr), *(yyvsp[(3) - (3)].boolExpr));
    ;}
    break;

  case 38:

/* Line 1455 of yacc.c  */
#line 545 "../src/C.y"
    {
      YYSTYPE::VariableSpec vSpec = {false,false,false,false,false,false,false,false,""};
      vSpec.isGlobal = true;
      (yyval.varSpec) = vSpec;   
   ;}
    break;

  case 39:

/* Line 1455 of yacc.c  */
#line 551 "../src/C.y"
    {
      YYSTYPE::VariableSpec vSpec = {false,false,false,false,false,false,false,false,""};
      vSpec.isLocal = true;
      (yyval.varSpec) = vSpec;   
   ;}
    break;

  case 40:

/* Line 1455 of yacc.c  */
#line 557 "../src/C.y"
    {
      YYSTYPE::VariableSpec vSpec = {false,false,false,false,false,false,false,false,""};
      vSpec.isParam = true;
      (yyval.varSpec) = vSpec;   
   ;}
    break;

  case 41:

/* Line 1455 of yacc.c  */
#line 566 "../src/C.y"
    {
       (yyval.snippet) = snippetGen->findInstVariable(dynC_API::getMangledStub((yyvsp[(1) - (1)].sval), dynCSnippetName).c_str(), (yyvsp[(1) - (1)].sval));
       if((yyval.snippet) == NULL){
          (yyval.snippet) = new BPatch_nullExpr();
          yyerror(snippetGen->getError().c_str());
          break;
       }
    ;}
    break;

  case 42:

/* Line 1455 of yacc.c  */
#line 575 "../src/C.y"
    {
       (yyval.snippet) = snippetGen->findAppVariable((yyvsp[(3) - (3)].sval));
       if((yyval.snippet) == NULL){
          (yyval.snippet) = new BPatch_nullExpr();
          yyerrorNoTokNonUni(snippetGen->getError().c_str());
          break;
       }
    ;}
    break;

  case 43:

/* Line 1455 of yacc.c  */
#line 585 "../src/C.y"
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
             yyerror(snippetGen->getError().c_str());
             YYABORT;
          }else{
             yyerrorNonUni(snippetGen->getError().c_str());
             YYABORT;
          }
          break;
       }
    ;}
    break;

  case 44:

/* Line 1455 of yacc.c  */
#line 611 "../src/C.y"
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
          yyerrorNoTokNonUni(snippetGen->getError().c_str());
          break;
       }
    ;}
    break;

  case 45:

/* Line 1455 of yacc.c  */
#line 631 "../src/C.y"
    {
       //array referance
       //check for integer in arith_expression
       (yyval.snippet) = snippetGen->generateArrayRef((yyvsp[(1) - (4)].snippet), (yyvsp[(3) - (4)].snippet));
       if((yyval.snippet) == NULL){
          (yyval.snippet) = new BPatch_nullExpr();
          yyerror(snippetGen->getError().c_str());
          break;
       }
    ;}
    break;

  case 46:

/* Line 1455 of yacc.c  */
#line 642 "../src/C.y"
    {
       (yyval.snippet) = (BPatch_snippet *)(new BPatch_arithExpr(BPatch_deref, *(yyvsp[(2) - (2)].snippet)));
    ;}
    break;

  case 47:

/* Line 1455 of yacc.c  */
#line 646 "../src/C.y"
    {
       (yyval.snippet) = (BPatch_snippet *)(new BPatch_arithExpr(BPatch_addr, *(yyvsp[(2) - (2)].snippet)));
    ;}
    break;

  case 48:

/* Line 1455 of yacc.c  */
#line 653 "../src/C.y"
    { 
      if(verbose) printf(" %d ", (yyvsp[(1) - (1)].ival));
      BPatch_snippet * c = new BPatch_constExpr((yyvsp[(1) - (1)].ival));
      (yyval.snippetStringPair) = new std::pair<BPatch_snippet *, const char *>(c, "int");
   ;}
    break;

  case 49:

/* Line 1455 of yacc.c  */
#line 659 "../src/C.y"
    { 
       if(verbose) printf(" %s ", (yyvsp[(1) - (1)].sval));
       BPatch_snippet * c = new BPatch_constExpr((yyvsp[(1) - (1)].sval));
       (yyval.snippetStringPair) = new std::pair<BPatch_snippet *, const char *>(c, "char *");
    ;}
    break;

  case 50:

/* Line 1455 of yacc.c  */
#line 668 "../src/C.y"
    {
        std::vector<std::pair<BPatch_snippet *, const char *> > *cnlist = new std::vector<std::pair<BPatch_snippet *, const char *> >();
        cnlist->push_back(*(yyvsp[(1) - (1)].snippetStringPair));
        (yyval.snippetStringListPair) = cnlist;
     ;}
    break;

  case 51:

/* Line 1455 of yacc.c  */
#line 674 "../src/C.y"
    {
        (yyvsp[(1) - (3)].snippetStringListPair)->push_back(*(yyvsp[(3) - (3)].snippetStringPair));
        (yyval.snippetStringListPair) = (yyvsp[(1) - (3)].snippetStringListPair);
     ;}
    break;

  case 53:

/* Line 1455 of yacc.c  */
#line 680 "../src/C.y"
    {(yyval.snippet) = (yyvsp[(1) - (1)].snippetStringPair)->first;;}
    break;

  case 54:

/* Line 1455 of yacc.c  */
#line 681 "../src/C.y"
    {(yyval.snippet) = new BPatch_nullExpr();;}
    break;

  case 55:

/* Line 1455 of yacc.c  */
#line 683 "../src/C.y"
    {
   (yyval.snippet) = snippetGen->findRegister((yyvsp[(3) - (3)].sval));
   if ((yyval.snippet) == NULL){
      (yyval.snippet) = new BPatch_nullExpr();
      yyerror(snippetGen->getError().c_str());
   }
;}
    break;

  case 56:

/* Line 1455 of yacc.c  */
#line 692 "../src/C.y"
    {
       if(verbose) printf("dyninst`%s ", (yyvsp[(3) - (3)].sval));
       
       std::vector<BPatch_snippet *> argVect;
       //snippets w/ return vals
       if(strcmp((yyvsp[(3) - (3)].sval), "function_name") == 0){
          (yyval.snippet) = snippetGen->getContextInfo(SnippetGenerator::SG_FunctionName);
          if((yyval.snippet) == NULL){
             (yyval.snippet) = new BPatch_nullExpr();
             yyerror(snippetGen->getError().c_str());
          }
          break;
       }
       if(strcmp((yyvsp[(3) - (3)].sval), "module_name") == 0){
          (yyval.snippet) = snippetGen->getContextInfo(SnippetGenerator::SG_ModuleName);
          if((yyval.snippet) == NULL){
             (yyval.snippet) = new BPatch_nullExpr();
             yyerror(snippetGen->getError().c_str());
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
             yyerror(snippetGen->getError().c_str());
          }
          break;
       }
       if(strcmp((yyvsp[(3) - (3)].sval), "dynamic_target") == 0){
          (yyval.snippet) = new BPatch_dynamicTargetExpr();
          break;
       }
 
       yyerror("Syntax error: unrecognized dyninst call");
       (yyval.snippet) = new BPatch_nullExpr();
    ;}
    break;

  case 57:

/* Line 1455 of yacc.c  */
#line 760 "../src/C.y"
    {
       if(verbose) printf(" * ");
       (yyval.snippet) = new BPatch_arithExpr(BPatch_times, *(yyvsp[(1) - (3)].snippet), *(yyvsp[(3) - (3)].snippet));
       actionTaken = true;
    ;}
    break;

  case 58:

/* Line 1455 of yacc.c  */
#line 766 "../src/C.y"
    {
       (yyval.snippet) = (yyvsp[(1) - (1)].snippet);
       actionTaken = true;
    ;}
    break;

  case 59:

/* Line 1455 of yacc.c  */
#line 771 "../src/C.y"
    {
       if(verbose) printf(" = ");
	    (yyval.snippet) = new BPatch_arithExpr(BPatch_assign, *(yyvsp[(1) - (3)].snippet), *(yyvsp[(3) - (3)].snippet));
       actionTaken = true;
    ;}
    break;

  case 60:

/* Line 1455 of yacc.c  */
#line 777 "../src/C.y"
    {
       if(verbose) printf(" += ");
       (yyval.snippet) = new BPatch_arithExpr(BPatch_assign, *(yyvsp[(1) - (3)].snippet), BPatch_arithExpr(BPatch_plus, *(yyvsp[(1) - (3)].snippet), *(yyvsp[(3) - (3)].snippet)));
       actionTaken = true;
    ;}
    break;

  case 61:

/* Line 1455 of yacc.c  */
#line 783 "../src/C.y"
    {
       if(verbose) printf(" -= ");
       (yyval.snippet) = new BPatch_arithExpr(BPatch_assign, *(yyvsp[(1) - (3)].snippet), BPatch_arithExpr(BPatch_minus, *(yyvsp[(1) - (3)].snippet), *(yyvsp[(3) - (3)].snippet)));
       actionTaken = true;
    ;}
    break;

  case 62:

/* Line 1455 of yacc.c  */
#line 789 "../src/C.y"
    {
       if(verbose) printf(" *= ");
       (yyval.snippet) = new BPatch_arithExpr(BPatch_assign, *(yyvsp[(1) - (3)].snippet), BPatch_arithExpr(BPatch_times, *(yyvsp[(1) - (3)].snippet), *(yyvsp[(3) - (3)].snippet)));
       actionTaken = true;
    ;}
    break;

  case 63:

/* Line 1455 of yacc.c  */
#line 795 "../src/C.y"
    {
       if(verbose) printf(" /= ");
       (yyval.snippet) = new BPatch_arithExpr(BPatch_assign, *(yyvsp[(1) - (3)].snippet), BPatch_arithExpr(BPatch_divide, *(yyvsp[(1) - (3)].snippet), *(yyvsp[(3) - (3)].snippet)));
       actionTaken = true;
    ;}
    break;

  case 64:

/* Line 1455 of yacc.c  */
#line 801 "../src/C.y"
    {
       if(verbose) printf(" %%= ");
       (yyval.snippet) = new BPatch_arithExpr(BPatch_assign, *(yyvsp[(1) - (3)].snippet), BPatch_arithExpr(BPatch_minus, *(yyvsp[(1) - (3)].snippet), BPatch_arithExpr(BPatch_times, BPatch_arithExpr(BPatch_divide, *(yyvsp[(1) - (3)].snippet), *(yyvsp[(3) - (3)].snippet)), *(yyvsp[(3) - (3)].snippet))));
       actionTaken = true;
    ;}
    break;

  case 65:

/* Line 1455 of yacc.c  */
#line 807 "../src/C.y"
    {
       if(verbose) printf(" / ");
       (yyval.snippet) = new BPatch_arithExpr(BPatch_divide, *(yyvsp[(1) - (3)].snippet), *(yyvsp[(3) - (3)].snippet));
       actionTaken = true;
    ;}
    break;

  case 66:

/* Line 1455 of yacc.c  */
#line 813 "../src/C.y"
    {
       if(verbose) printf(" %% ");
       (yyval.snippet) = new BPatch_arithExpr(BPatch_minus, *(yyvsp[(1) - (3)].snippet), BPatch_arithExpr(BPatch_times, BPatch_arithExpr(BPatch_divide, *(yyvsp[(1) - (3)].snippet), *(yyvsp[(3) - (3)].snippet)), *(yyvsp[(3) - (3)].snippet)));
       actionTaken = true;
    ;}
    break;

  case 67:

/* Line 1455 of yacc.c  */
#line 819 "../src/C.y"
    {
       if(verbose) printf(" + ");
       (yyval.snippet) = new BPatch_arithExpr(BPatch_plus, *(yyvsp[(1) - (3)].snippet), *(yyvsp[(3) - (3)].snippet));
       actionTaken = true;
    ;}
    break;

  case 68:

/* Line 1455 of yacc.c  */
#line 825 "../src/C.y"
    {
       if(verbose) printf(" - ");
       (yyval.snippet) = new BPatch_arithExpr(BPatch_minus, *(yyvsp[(1) - (3)].snippet), *(yyvsp[(3) - (3)].snippet));
       actionTaken = true;
    ;}
    break;

  case 69:

/* Line 1455 of yacc.c  */
#line 831 "../src/C.y"
    {
       if(dynamic_cast<BPatch_nullExpr *>((yyvsp[(1) - (3)].snippet))){
          printf("Picked second\n");
          (yyval.snippet) = (yyvsp[(3) - (3)].snippet);
       }else{
          (yyval.snippet) = (yyvsp[(1) - (3)].snippet);
       }
    ;}
    break;

  case 70:

/* Line 1455 of yacc.c  */
#line 839 "../src/C.y"
    {(yyval.snippet) = (yyvsp[(2) - (3)].snippet);;}
    break;

  case 71:

/* Line 1455 of yacc.c  */
#line 841 "../src/C.y"
    {
       (yyval.snippet) = (yyvsp[(1) - (1)].snippet);
       actionTaken = true;
    ;}
    break;

  case 72:

/* Line 1455 of yacc.c  */
#line 850 "../src/C.y"
    {
       if(verbose) printf(" ++ ");
       BPatch_arithExpr addOne = BPatch_arithExpr(BPatch_assign, *(yyvsp[(1) - (2)].snippet), BPatch_arithExpr(BPatch_plus, *(yyvsp[(1) - (2)].snippet), BPatch_constExpr(1)));
       (yyval.snippet) = new BPatch_arithExpr(BPatch_seq, (BPatch_snippet &)addOne, BPatch_arithExpr(BPatch_minus, *(yyvsp[(1) - (2)].snippet), BPatch_constExpr(1)));
    ;}
    break;

  case 73:

/* Line 1455 of yacc.c  */
#line 856 "../src/C.y"
    {
       if(verbose) printf(" ++ ");
       BPatch_arithExpr addOne = BPatch_arithExpr(BPatch_assign, *(yyvsp[(2) - (2)].snippet), BPatch_arithExpr(BPatch_plus, *(yyvsp[(2) - (2)].snippet), BPatch_constExpr(1)));
       (yyval.snippet) = new BPatch_arithExpr(BPatch_seq, (BPatch_snippet &)addOne, *(yyvsp[(2) - (2)].snippet));
    ;}
    break;

  case 74:

/* Line 1455 of yacc.c  */
#line 862 "../src/C.y"
    {
       if(verbose) printf(" -- ");
       BPatch_arithExpr subOne = BPatch_arithExpr(BPatch_assign, *(yyvsp[(1) - (2)].snippet), BPatch_arithExpr(BPatch_minus, *(yyvsp[(1) - (2)].snippet), BPatch_constExpr(1)));
       (yyval.snippet) = new BPatch_arithExpr(BPatch_seq, (BPatch_snippet &)subOne, BPatch_arithExpr(BPatch_plus, *(yyvsp[(1) - (2)].snippet), BPatch_constExpr(1)));
    ;}
    break;

  case 75:

/* Line 1455 of yacc.c  */
#line 868 "../src/C.y"
    {
       if(verbose) printf(" -- ");
       BPatch_arithExpr subOne = BPatch_arithExpr(BPatch_assign, *(yyvsp[(2) - (2)].snippet), BPatch_arithExpr(BPatch_minus, *(yyvsp[(2) - (2)].snippet), BPatch_constExpr(1)));
       (yyval.snippet) = new BPatch_arithExpr(BPatch_seq, (BPatch_snippet &)subOne, *(yyvsp[(2) - (2)].snippet));
    ;}
    break;



/* Line 1455 of yacc.c  */
#line 2800 "dynC.tab.c"
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
      /* If just tried and failed to reuse lookahead token after an
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

  /* Else will try to reuse lookahead token after shifting the error
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

  *++yyvsp = yylval;

  yyerror_range[1] = yylloc;
  /* Using YYLLOC is tempting, but would change the location of
     the lookahead.  YYLOC is available though.  */
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

#if !defined(yyoverflow) || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
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



/* Line 1675 of yacc.c  */
#line 875 "../src/C.y"


#include <stdio.h>

std::stringstream * err;

void yyerror(const char *s)
{
   fatalError = true;
   fflush(stdout);
   if(strlen(s) != 0){
      std::stringstream err;
      err.str("");
      char ebase[256];
      getErrorBase(ebase, 256);
      err << ebase << " error: " << s << " for token '" << lineStr << "'";
      if(universalErrors->find(err.str()) != universalErrors->end()){
         return;
      }
      printf("%s\n", err.str().c_str());
      universalErrors->insert(err.str());
   }
}

void yyerrorNonUni(const char *s){
   fatalError = true;
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

void yyerrorNoTok(const char *s){
   fatalError = true;
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
   universalErrors->insert(err.str());
}


void yyerrorNoTokNonUni(const char *s){
   fatalError = true;
   char ebase[256];
   getErrorBase(ebase, 256);
   printf("%s error: %s\n", ebase, s);
}

void yywarn(const char *s)
{
   std::stringstream err;
   err.str("");
   err << dynCSnippetName << ":" << yylloc.first_line << ":" << " warning: " << s;
   if(universalErrors->find(err.str()) != universalErrors->end()){
      return;
   }   
   printf("%s\n", err.str().c_str());
   universalErrors->insert(err.str());
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

