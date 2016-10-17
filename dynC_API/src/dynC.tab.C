/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

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
#define YYBISON_VERSION "3.0.4"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1


/* Substitute the variable and function names.  */
#define yyparse         dynCparse
#define yylex           dynClex
#define yyerror         dynCerror
#define yydebug         dynCdebug
#define yynerrs         dynCnerrs

#define yylval          dynClval
#define yychar          dynCchar
#define yylloc          dynClloc

/* Copy the first part of user declarations.  */
#line 5 "C.y" /* yacc.c:339  */

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
   void yyerror(const char *s); 
   void yyerrorNonUni(const char *s);
   void yyerrorNoTok(const char *s);
   void yyerrorNoTokNonUni(const char *s);
   void yywarn(const char *s);
   int yyparse();
   void makeOneTimeStatement(BPatch_snippet &statement);
   void makeOneTimeStatementGbl(BPatch_snippet &statement);
   void getErrorBase(char *errbase, int length);
}

extern std::string lineStr;

#define YYDEBUG 0 //set to 1 for debug mode

// name of current snippet for error reporting
char *dynCSnippetName = "";

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

#line 137 "dynC.tab.C" /* yacc.c:339  */

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* In a future release of Bison, this section will be replaced
   by #include "dynC.tab.h".  */
#ifndef YY_DYNC_DYNC_TAB_H_INCLUDED
# define YY_DYNC_DYNC_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int dynCdebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    KNOWN_ERROR_TOK = 258,
    IDENTIFIER = 259,
    CONSTANT = 260,
    STRING = 261,
    TYPE = 262,
    NUMBER = 263,
    D_ERROR = 264,
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
    D_CHAR = 283,
    D_SHORT = 284,
    D_INT = 285,
    D_LONG = 286,
    SIGNED = 287,
    UNSIGNED = 288,
    D_FLOAT = 289,
    DOUBLE = 290,
    D_CONST = 291,
    D_VOID = 292,
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
    DOT = 312,
    ASTERISK = 313,
    AMPERSAND = 314,
    COMMA = 315,
    NOT = 316,
    OR = 317,
    AND = 318,
    LESS_EQ = 319,
    GREATER_EQ = 320,
    EQ = 321,
    NOT_EQ = 322,
    COLON = 323,
    SEMI = 324,
    START_BLOCK = 325,
    END_BLOCK = 326,
    DOLLAR = 327,
    BACKTICK = 328,
    AND_OP = 329,
    OR_OP = 330,
    MUL_ASSIGN = 331,
    DIV_ASSIGN = 332,
    MOD_ASSIGN = 333,
    ADD_ASSIGN = 334,
    SUB_ASSIGN = 335,
    ASSIGN = 336,
    NOPEN = 337,
    NCLOSE = 338,
    LOWER_THAN_ELSE = 339,
    ELSE = 340,
    LOWER_THAN_DEREF = 341
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 68 "C.y" /* yacc.c:355  */

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

#line 294 "dynC.tab.C" /* yacc.c:355  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif


extern YYSTYPE dynClval;
extern YYLTYPE dynClloc;
int dynCparse (void);

#endif /* !YY_DYNC_DYNC_TAB_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 325 "dynC.tab.C" /* yacc.c:358  */

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
#else
typedef signed char yytype_int8;
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
# elif ! defined YYSIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
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
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
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
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
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

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  45
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   498

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  104
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  17
/* YYNRULES -- Number of rules.  */
#define YYNRULES  75
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  147

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   341

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
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
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   166,   166,   189,   209,   227,   258,   291,   339,   344,
     357,   363,   368,   376,   386,   391,   397,   402,   413,   421,
     432,   434,   441,   456,   471,   476,   482,   491,   495,   502,
     503,   508,   513,   518,   523,   528,   533,   538,   546,   552,
     558,   567,   576,   586,   612,   632,   643,   647,   654,   660,
     669,   675,   681,   682,   683,   684,   693,   761,   767,   772,
     778,   784,   790,   796,   802,   808,   814,   820,   826,   832,
     841,   842,   851,   857,   863,   869
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "KNOWN_ERROR_TOK", "IDENTIFIER",
  "CONSTANT", "STRING", "TYPE", "NUMBER", "D_ERROR", "EOL", "SIZEOF",
  "D_TRUE", "D_FALSE", "PTR_OP", "INC_OP", "DEC_OP", "LEFT_OP", "RIGHT_OP",
  "LEFT_ASSIGN", "RIGHT_ASSIGN", "AND_ASSIGN", "XOR_ASSIGN", "OR_ASSIGN",
  "TYPE_NAME", "TYPEDEF", "EXTERN", "STATIC", "D_CHAR", "D_SHORT", "D_INT",
  "D_LONG", "SIGNED", "UNSIGNED", "D_FLOAT", "DOUBLE", "D_CONST", "D_VOID",
  "STRUCT", "UNION", "ENUM", "ELLIPSIS", "IF", "LOCAL", "PARAM", "GLOBAL",
  "FUNC", "DYNINST", "INST", "REGISTER", "NEWLINE", "CASE", "DEFAULT",
  "SWITCH", "RETURN", "NILL", "EOF_TOK", "DOT", "ASTERISK", "AMPERSAND",
  "COMMA", "NOT", "OR", "AND", "LESS_EQ", "GREATER_EQ", "EQ", "NOT_EQ",
  "'+'", "'-'", "'*'", "'/'", "'%'", "'|'", "COLON", "SEMI", "START_BLOCK",
  "END_BLOCK", "DOLLAR", "BACKTICK", "'['", "']'", "'!'", "'~'", "'&'",
  "AND_OP", "OR_OP", "MUL_ASSIGN", "DIV_ASSIGN", "MOD_ASSIGN",
  "ADD_ASSIGN", "SUB_ASSIGN", "ASSIGN", "NOPEN", "NCLOSE",
  "LOWER_THAN_ELSE", "ELSE", "LOWER_THAN_DEREF", "'{'", "'}'", "'('",
  "')'", "'<'", "'>'", "$accept", "start", "var_declaration",
  "var_decl_modifiers", "statement_list", "statement", "block",
  "func_call", "param_list", "bool_constant", "bool_expression",
  "var_modifiers", "variable_expr", "constant", "const_list",
  "arith_expression", "inc_decr_expr", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
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

#define YYPACT_NINF -91

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-91)))

#define YYTABLE_NINF -3

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     287,   -91,   -91,   -91,   -91,   -91,   -91,    23,    23,    -4,
     -90,   -91,   -91,   -91,   -57,   -48,    -9,   -91,    23,    23,
     287,   398,    26,   -20,    67,    45,   -91,   -91,    -6,   337,
     -91,    71,   -91,    -5,    15,    15,   -91,   352,    92,    94,
      95,    15,   -91,   110,    62,   -91,   -91,   -71,   287,   -91,
       0,   -91,   -91,   398,   398,   398,   398,   398,   398,   398,
     398,   398,   398,   398,   398,   398,   -91,    97,   -91,   -91,
     -91,   -51,   390,     2,     3,   -91,   -91,   -91,    -3,   398,
     169,   -91,   -91,   118,   -91,   -91,   -91,   -91,   -91,   -91,
      -8,    -8,    36,    36,    36,   -91,   -91,   352,   352,   339,
     398,   398,   398,   398,   398,   398,   398,   398,    32,    28,
     177,   -91,   -91,    58,   -91,   287,   -91,    27,   177,   177,
     177,   177,   177,   177,   -54,   177,   -53,    30,    29,   228,
     339,   398,   -91,   -91,    38,     9,   -91,   -91,   177,     9,
     -91,   -42,   -40,     9,   -91,   -91,   -91
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
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
       0,    43,    44,     0,    62,    63,    64,    60,    61,    59,
      67,    68,    57,    65,    66,    69,    42,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    24,    24,     0,     0,
       4,    13,    45,    37,    36,     0,    20,    18,    33,    34,
      32,    35,    30,    31,     0,    25,     0,     5,     0,     0,
       0,     0,    23,    22,     0,     0,    21,    19,    26,     0,
      50,     0,     0,     0,     6,     7,    51
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -91,   -91,   -91,   115,   -18,   -24,    -2,   -91,    40,   -91,
     -69,   -91,     6,   -31,    10,   -21,   -91
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    22,    23,    24,    25,    26,   117,    27,   124,    70,
      71,    28,    29,    30,   141,    31,    32
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      44,    49,    43,     4,    81,   108,   131,   131,    82,    78,
      37,    97,    98,    34,    35,     3,    72,     5,   143,    49,
     143,    79,    38,     9,    41,    42,    45,     2,   113,   114,
      80,    39,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    -2,     1,   132,   133,     2,
      99,     3,     4,     5,     6,    46,    49,   144,   110,   145,
       7,     8,    62,    63,    64,    65,    11,    12,    13,    33,
      40,    47,     9,    50,    67,   116,    72,    72,   109,   118,
     119,   120,   121,   122,   123,   125,   125,    10,    11,    12,
      13,    14,    15,    18,    16,    53,    73,   129,    74,    75,
      17,    96,   106,   107,   140,    49,   116,    19,   140,    65,
     138,     1,   146,   127,     2,    18,     3,     4,     5,     6,
     128,    98,   134,   130,    36,     7,     8,   135,   137,    19,
      60,    61,    62,    63,    64,    65,   139,     9,    48,    60,
      61,    62,    63,    64,    65,    21,    66,   126,     0,   142,
       0,     0,    10,    11,    12,    13,    14,    15,     0,    16,
       0,     0,     0,    77,     0,    17,     0,     0,     0,     0,
       1,     0,     0,     2,     0,     3,     4,     5,     6,     0,
      18,     0,     0,     0,     7,     8,    60,    61,    62,    63,
      64,    65,     0,     0,    19,     0,     9,     0,     0,   112,
       0,     0,     0,    48,    76,     0,     0,     0,     0,     0,
      21,    10,    11,    12,    13,    14,    15,     0,    16,     0,
       0,     0,     0,     0,    17,     0,     0,     0,     0,     1,
       0,     0,     2,     0,     3,     4,     5,     6,     0,    18,
       0,     0,     0,     7,     8,    60,    61,    62,    63,    64,
      65,     0,     0,    19,     0,     9,     0,     0,     0,     0,
       0,     0,    48,   111,     0,     0,     0,     0,     0,    21,
      10,    11,    12,    13,    14,    15,     0,    16,     0,     0,
       0,     0,     0,    17,     0,     0,     0,     0,     1,     0,
       0,     2,     0,     3,     4,     5,     6,     0,    18,     0,
       0,     0,     7,     8,     0,     0,     0,     0,     0,     0,
       0,     0,    19,     0,     9,     0,     0,     0,     0,     0,
       0,    48,     0,     0,     0,     0,     0,   136,    21,    10,
      11,    12,    13,    14,    15,     0,    16,     0,     0,     0,
       1,     0,    17,     2,     0,     3,     4,     5,     6,     0,
       0,     0,    51,    52,     7,     8,     2,    18,     3,     0,
       5,     0,     0,     0,    68,    69,     9,     7,     8,     0,
       0,    19,     0,     0,     0,     0,     0,     0,     0,     0,
      20,    10,    11,    12,    13,    14,    15,    21,    16,     0,
       0,     0,     0,     0,    17,    11,    12,    13,    14,    15,
       0,    16,     2,     0,     3,     0,     5,    17,     0,    18,
       0,     0,     0,     7,     8,     0,     0,    53,     0,     0,
       0,     0,    18,    19,    54,    55,    56,    57,    58,    59,
       0,     0,     0,     0,     0,     0,    19,   115,     0,    21,
       0,    11,    12,    13,    14,    15,     0,    16,     0,     0,
       0,     0,    21,    17,   100,   101,   102,   103,    60,    61,
      62,    63,    64,    65,     0,     0,     0,     0,    18,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    19,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   104,   105,     0,     0,     0,     0,    21
};

static const yytype_int16 yycheck[] =
{
      21,    25,    20,     7,     4,     8,    60,    60,     8,    80,
     100,    62,    63,     7,     8,     6,    37,     8,    60,    43,
      60,    92,    79,    27,    18,    19,     0,     4,    97,    98,
      48,    79,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,     0,     1,   101,   101,     4,
     101,     6,     7,     8,     9,    75,    80,    99,    79,    99,
      15,    16,    70,    71,    72,    73,    43,    44,    45,    46,
      79,     4,    27,    79,    79,    99,    97,    98,    81,   100,
     101,   102,   103,   104,   105,   106,   107,    42,    43,    44,
      45,    46,    47,    70,    49,    80,     4,   115,     4,     4,
      55,     4,   100,   100,   135,   129,   130,    84,   139,    73,
     131,     1,   143,    81,     4,    70,     6,     7,     8,     9,
      92,    63,    92,    96,     9,    15,    16,    98,   130,    84,
      68,    69,    70,    71,    72,    73,    98,    27,    93,    68,
      69,    70,    71,    72,    73,   100,    75,   107,    -1,   139,
      -1,    -1,    42,    43,    44,    45,    46,    47,    -1,    49,
      -1,    -1,    -1,   101,    -1,    55,    -1,    -1,    -1,    -1,
       1,    -1,    -1,     4,    -1,     6,     7,     8,     9,    -1,
      70,    -1,    -1,    -1,    15,    16,    68,    69,    70,    71,
      72,    73,    -1,    -1,    84,    -1,    27,    -1,    -1,    81,
      -1,    -1,    -1,    93,    94,    -1,    -1,    -1,    -1,    -1,
     100,    42,    43,    44,    45,    46,    47,    -1,    49,    -1,
      -1,    -1,    -1,    -1,    55,    -1,    -1,    -1,    -1,     1,
      -1,    -1,     4,    -1,     6,     7,     8,     9,    -1,    70,
      -1,    -1,    -1,    15,    16,    68,    69,    70,    71,    72,
      73,    -1,    -1,    84,    -1,    27,    -1,    -1,    -1,    -1,
      -1,    -1,    93,    94,    -1,    -1,    -1,    -1,    -1,   100,
      42,    43,    44,    45,    46,    47,    -1,    49,    -1,    -1,
      -1,    -1,    -1,    55,    -1,    -1,    -1,    -1,     1,    -1,
      -1,     4,    -1,     6,     7,     8,     9,    -1,    70,    -1,
      -1,    -1,    15,    16,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    84,    -1,    27,    -1,    -1,    -1,    -1,    -1,
      -1,    93,    -1,    -1,    -1,    -1,    -1,    99,   100,    42,
      43,    44,    45,    46,    47,    -1,    49,    -1,    -1,    -1,
       1,    -1,    55,     4,    -1,     6,     7,     8,     9,    -1,
      -1,    -1,    15,    16,    15,    16,     4,    70,     6,    -1,
       8,    -1,    -1,    -1,    12,    13,    27,    15,    16,    -1,
      -1,    84,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      93,    42,    43,    44,    45,    46,    47,   100,    49,    -1,
      -1,    -1,    -1,    -1,    55,    43,    44,    45,    46,    47,
      -1,    49,     4,    -1,     6,    -1,     8,    55,    -1,    70,
      -1,    -1,    -1,    15,    16,    -1,    -1,    80,    -1,    -1,
      -1,    -1,    70,    84,    87,    88,    89,    90,    91,    92,
      -1,    -1,    -1,    -1,    -1,    -1,    84,    98,    -1,   100,
      -1,    43,    44,    45,    46,    47,    -1,    49,    -1,    -1,
      -1,    -1,   100,    55,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    -1,    -1,    -1,    -1,    70,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    84,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   102,   103,    -1,    -1,    -1,    -1,   100
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,     4,     6,     7,     8,     9,    15,    16,    27,
      42,    43,    44,    45,    46,    47,    49,    55,    70,    84,
      93,   100,   105,   106,   107,   108,   109,   111,   115,   116,
     117,   119,   120,    46,   116,   116,   107,   100,    79,    79,
      79,   116,   116,   108,   119,     0,    75,     4,    93,   109,
      79,    15,    16,    80,    87,    88,    89,    90,    91,    92,
      68,    69,    70,    71,    72,    73,    75,    79,    12,    13,
     113,   114,   119,     4,     4,     4,    94,   101,    80,    92,
     108,     4,     8,   119,   119,   119,   119,   119,   119,   119,
     119,   119,   119,   119,   119,   119,     4,    62,    63,   101,
      64,    65,    66,    67,   102,   103,   100,   100,     8,    81,
     119,    94,    81,   114,   114,    98,   109,   110,   119,   119,
     119,   119,   119,   119,   112,   119,   112,    81,    92,   108,
      96,    60,   101,   101,    92,    98,    99,   110,   119,    98,
     117,   118,   118,    60,    99,    99,   117
};

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

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
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


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)                                \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;        \
          (Current).first_column = YYRHSLOC (Rhs, 1).first_column;      \
          (Current).last_line    = YYRHSLOC (Rhs, N).last_line;         \
          (Current).last_column  = YYRHSLOC (Rhs, N).last_column;       \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).first_line   = (Current).last_line   =              \
            YYRHSLOC (Rhs, 0).last_line;                                \
          (Current).first_column = (Current).last_column =              \
            YYRHSLOC (Rhs, 0).last_column;                              \
        }                                                               \
    while (0)
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K])


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL

/* Print *YYLOCP on YYO.  Private, do not rely on its existence. */

YY_ATTRIBUTE_UNUSED
static unsigned
yy_location_print_ (FILE *yyo, YYLTYPE const * const yylocp)
{
  unsigned res = 0;
  int end_col = 0 != yylocp->last_column ? yylocp->last_column - 1 : 0;
  if (0 <= yylocp->first_line)
    {
      res += YYFPRINTF (yyo, "%d", yylocp->first_line);
      if (0 <= yylocp->first_column)
        res += YYFPRINTF (yyo, ".%d", yylocp->first_column);
    }
  if (0 <= yylocp->last_line)
    {
      if (yylocp->first_line < yylocp->last_line)
        {
          res += YYFPRINTF (yyo, "-%d", yylocp->last_line);
          if (0 <= end_col)
            res += YYFPRINTF (yyo, ".%d", end_col);
        }
      else if (0 <= end_col && yylocp->first_column < end_col)
        res += YYFPRINTF (yyo, "-%d", end_col);
    }
  return res;
 }

#  define YY_LOCATION_PRINT(File, Loc)          \
  yy_location_print_ (File, &(Loc))

# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value, Location); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  YYUSE (yylocationp);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  YY_LOCATION_PRINT (yyoutput, *yylocationp);
  YYFPRINTF (yyoutput, ": ");
  yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, YYLTYPE *yylsp, int yyrule)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                       , &(yylsp[(yyi + 1) - (yynrhs)])                       );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, yylsp, Rule); \
} while (0)

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
#ifndef YYINITDEPTH
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
static YYSIZE_T
yystrlen (const char *yystr)
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
static char *
yystpcpy (char *yydest, const char *yysrc)
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

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, YYLTYPE *yylocationp)
{
  YYUSE (yyvaluep);
  YYUSE (yylocationp);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Location data for the lookahead symbol.  */
YYLTYPE yylloc
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
  = { 1, 1, 1, 1 }
# endif
;
/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.
       'yyls': related to locations.

       Refer to the stacks through separate pointers, to allow yyoverflow
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
    YYLTYPE yyerror_range[3];

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
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

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yylsp = yyls = yylsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

/* User initialization code.  */
#line 98 "C.y" /* yacc.c:1429  */
{
#ifdef YYDEBUG
#if YYDEBUG == 1
   yydebug = 1;
#endif
#endif
 }

#line 1508 "dynC.tab.C" /* yacc.c:1429  */
  yylsp[0] = yylloc;
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
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex ();
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
      if (yytable_value_is_error (yyn))
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
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END
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
     '$$ = $1'.

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
#line 167 "C.y" /* yacc.c:1646  */
    { 
       oneTimeCount = 0;
       oneTimeGblCount = 0;
       if(fatalError){
          parse_result = NULL;
       }else{
          (yyvsp[0].snippetList)->insert((yyvsp[0].snippetList)->end(), endSnippets.begin(), endSnippets.end());
          parse_result = new BPatch_sequence(*(yyvsp[0].snippetList));
       }

       fatalError = false;
       delete (yyvsp[0].snippetList);

       endSnippets.clear();
       if(verbose) {
          printf("\n");
          fflush(stdout);
       }
       YYACCEPT;
    }
#line 1716 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 3:
#line 190 "C.y" /* yacc.c:1646  */
    {
       std::string mangledName;
       mangledName = dynC_API::mangle((yyvsp[0].sval), dynCSnippetName, (yyvsp[-1].varSpec).type);
       if(verbose) printf("name : %s\n", mangledName.c_str());
       (yyval.snippet) = snippetGen->findOrCreateVariable(mangledName.c_str(), (yyvsp[-1].varSpec).type);
       if((yyval.snippet) == NULL){
          (yyval.snippet) = new BPatch_nullExpr();
          yyerror(snippetGen->getError().c_str());
          break;
       }

        if(!((yyvsp[-1].varSpec).isStatic || (yyvsp[-1].varSpec).isGlobal)){
          BPatch_snippet setSn = BPatch_constExpr(0); // if int etc.
          if(strcmp((yyvsp[-1].varSpec).type, "char *") == 0){
             setSn = BPatch_constExpr("");
          }
          endSnippets.push_back(new BPatch_arithExpr(BPatch_assign, *(yyval.snippet), setSn));
          }
    }
#line 1740 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 4:
#line 210 "C.y" /* yacc.c:1646  */
    {   
      
       std::string mangledName = dynC_API::mangle((yyvsp[-2].sval), dynCSnippetName, (yyvsp[-3].varSpec).type);
       if(verbose) printf("name : %s\n", mangledName.c_str());
       BPatch_snippet *alloc = snippetGen->findOrCreateVariable(mangledName.c_str(), (yyvsp[-3].varSpec).type);
       if(alloc == NULL){
          (yyval.snippet) = new BPatch_nullExpr();
          yyerror(snippetGen->getError().c_str());
          break;
       }
       BPatch_arithExpr *assign = new BPatch_arithExpr(BPatch_assign, *alloc, *(yyvsp[0].snippet));
       (yyval.snippet) = assign;

       if((yyvsp[-3].varSpec).isStatic || (yyvsp[-3].varSpec).isGlobal){
          makeOneTimeStatementGbl(*(yyval.snippet));
       }       
    }
#line 1762 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 5:
#line 228 "C.y" /* yacc.c:1646  */
    {
       //IDENTIFIER leaks, but how to fix b/c use of $0?
       if((yyvsp[-1].ival) < 0){
          std::stringstream errMessage;
          errMessage << "Invalid array size: " << (yyvsp[-1].ival);
          yyerrorNoTok(errMessage.str().c_str());
          (yyval.snippet) = new BPatch_nullExpr();
          break;
       }
      
       std::stringstream type;
       type << (yyvsp[-4].varSpec).type << "[" << (yyvsp[-1].ival) << "]";
       std::string mangledName = dynC_API::mangle((yyvsp[-3].sval), dynCSnippetName, type.str().c_str());
       (yyval.snippet) = snippetGen->findOrCreateArray(mangledName.c_str(), (yyvsp[-4].varSpec).type, (yyvsp[-1].ival));
       if((yyval.snippet) == NULL){
          (yyval.snippet) = new BPatch_nullExpr();
          yyerror(snippetGen->getError().c_str());
          break;
       }
       if(!((yyvsp[-4].varSpec).isStatic || (yyvsp[-4].varSpec).isGlobal)){
          BPatch_snippet setSn = BPatch_constExpr(0); // if int etc.
          if(strcmp((yyvsp[-4].varSpec).type, "char *") == 0){
             setSn = BPatch_constExpr("");
          }
          for(int n = 0; n < (yyvsp[-1].ival); ++n){
             endSnippets.push_back(new BPatch_arithExpr(BPatch_assign, BPatch_arithExpr(BPatch_ref, *(yyval.snippet), BPatch_constExpr(n)), setSn));             
          }
       }

    }
#line 1797 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 6:
#line 259 "C.y" /* yacc.c:1646  */
    {
       std::stringstream type;
       type << (yyvsp[-7].varSpec).type << "[" << (yyvsp[-1].snippetStringListPair)->size() << "]";
       std::string mangledName = dynC_API::mangle((yyvsp[-6].sval), dynCSnippetName, type.str().c_str());

       (yyval.snippet) = snippetGen->findOrCreateArray(mangledName.c_str(), (yyvsp[-7].varSpec).type, (yyvsp[-1].snippetStringListPair)->size()); //will only allocate memory if nessessary
       if((yyval.snippet) == NULL){
          (yyval.snippet) = new BPatch_nullExpr();
          yyerror(snippetGen->getError().c_str());
          break;
       }
       std::vector<BPatch_snippet* > *assignVect = new std::vector<BPatch_snippet *>();
       assignVect->push_back((yyval.snippet));
       for(unsigned int n = 0; n < (yyvsp[-1].snippetStringListPair)->size(); ++n){
          if(strcmp((yyvsp[-7].varSpec).type, (*(yyvsp[-1].snippetStringListPair))[n].second) != 0){
             std::string errMessage = "Type conflict when trying to initialize array of type \'";
             errMessage += type.str();
             errMessage += "\' with a value of type ";
             errMessage += (*(yyvsp[-1].snippetStringListPair))[n].second;
             yyerrorNoTok(errMessage.c_str());
             (yyval.snippet) = new BPatch_nullExpr();
             break;
          }
          BPatch_snippet *assign = new BPatch_arithExpr(BPatch_assign, BPatch_arithExpr(BPatch_ref, *(yyval.snippet), BPatch_constExpr(n)), *(*(yyvsp[-1].snippetStringListPair))[n].first);
          
          if((yyvsp[-7].varSpec).isStatic || (yyvsp[-7].varSpec).isGlobal){
              makeOneTimeStatement(*assign);
              }
          assignVect->push_back(assign);
       }
       (yyval.snippet) = new BPatch_sequence(*assignVect);
    }
#line 1834 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 7:
#line 292 "C.y" /* yacc.c:1646  */
    {
       std::vector<BPatch_snippet *> argVect;
       std::stringstream type;
       if((yyvsp[-5].ival) < 0){
          std::stringstream errMessage;
          errMessage << "Invalid array size: " << (yyvsp[-5].ival);
          yyerrorNoTok(errMessage.str().c_str());
          (yyval.snippet) = new BPatch_nullExpr();
          break;
       }
       type << (yyvsp[-8].varSpec).type << "[" << (yyvsp[-5].ival) << "]";
       std::string mangledName = dynC_API::mangle((yyvsp[-7].sval), dynCSnippetName, type.str().c_str());

       (yyval.snippet) = snippetGen->findOrCreateArray(mangledName.c_str(), (yyvsp[-8].varSpec).type, (yyvsp[-5].ival)); //will only allocate memory if nessessary
       if((yyval.snippet) == NULL){
          (yyval.snippet) = new BPatch_nullExpr();
          yyerror(snippetGen->getError().c_str());
          break;
       }
       std::vector<BPatch_snippet* > *assignVect = new std::vector<BPatch_snippet *>();
       assignVect->push_back((yyval.snippet));
       if((unsigned int)(yyvsp[-5].ival) != (yyvsp[-1].snippetStringListPair)->size()){
          yyerrorNoTok("Invalid number of arguments given in array initialization");
          (yyval.snippet) = new BPatch_nullExpr();
          break;
       }
       for(int n = 0; n < (yyvsp[-5].ival); ++n){
          if(strcmp((yyvsp[-8].varSpec).type, (*(yyvsp[-1].snippetStringListPair))[n].second) != 0){
             std::string errMessage = "Type conflict when trying to initialize array of type \'";
             errMessage += type.str();
             errMessage += "\' with a value of type ";
             errMessage += (*(yyvsp[-1].snippetStringListPair))[n].second;
             yyerrorNoTok(errMessage.c_str());
             (yyval.snippet) = new BPatch_nullExpr();
             break;
          }
          BPatch_snippet *assign = new BPatch_arithExpr(BPatch_assign, BPatch_arithExpr(BPatch_ref, *(yyval.snippet), BPatch_constExpr(n)), *(*(yyvsp[-1].snippetStringListPair))[n].first);
          if((yyvsp[-8].varSpec).isStatic || (yyvsp[-8].varSpec).isGlobal){
             makeOneTimeStatement(*assign);
          }
          assignVect->push_back(assign);
       
       }
       (yyval.snippet) = new BPatch_sequence(*assignVect);
    }
#line 1884 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 8:
#line 340 "C.y" /* yacc.c:1646  */
    {
      YYSTYPE::VariableSpec rSpec = {false,false,false,false,false,false,false,false,(yyvsp[0].sval)};
      (yyval.varSpec) = rSpec;
   }
#line 1893 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 9:
#line 345 "C.y" /* yacc.c:1646  */
    {
      if ((yyvsp[0].varSpec).isStatic){
         //throw error: two static
         yyerror("Syntax error");
      }else{
         (yyvsp[0].varSpec).isStatic = true;
      }
      (yyval.varSpec) = (yyvsp[0].varSpec);
  }
#line 1907 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 10:
#line 358 "C.y" /* yacc.c:1646  */
    { 
       if(verbose) printf("\n");
       (yyval.snippetList) = new BPatch_Vector<BPatch_snippet *>; 
       (yyval.snippetList)->push_back((yyvsp[0].snippet));
    }
#line 1917 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 11:
#line 364 "C.y" /* yacc.c:1646  */
    {
       (yyvsp[-1].snippetList)->push_back((yyvsp[0].snippet));
       (yyval.snippetList) = (yyvsp[-1].snippetList);
    }
#line 1926 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 12:
#line 369 "C.y" /* yacc.c:1646  */
    {
       BPatch_sequence *seq = new BPatch_sequence(*(yyvsp[-1].snippetList));
       makeOneTimeStatementGbl(*seq);
       std::vector<BPatch_snippet *> *retVect = new std::vector<BPatch_snippet *>;
       retVect->push_back(seq);
       (yyval.snippetList) = retVect;       
    }
#line 1938 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 13:
#line 377 "C.y" /* yacc.c:1646  */
    {
       BPatch_sequence seq = BPatch_sequence(*(yyvsp[-1].snippetList));
       makeOneTimeStatementGbl(seq);
       (yyvsp[-3].snippetList)->push_back(&seq);
       (yyval.snippetList) = (yyvsp[-3].snippetList);
    }
#line 1949 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 14:
#line 387 "C.y" /* yacc.c:1646  */
    {
       (yyval.snippet) = new BPatch_nullExpr();
       actionTaken = false;
    }
#line 1958 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 15:
#line 392 "C.y" /* yacc.c:1646  */
    {
       yyerrorNoTok((yyvsp[0].context));
       (yyval.snippet) = new BPatch_nullExpr();
       actionTaken = false;
    }
#line 1968 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 16:
#line 398 "C.y" /* yacc.c:1646  */
    {
       (yyval.snippet) = (yyvsp[-1].snippet);
    }
#line 1976 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 17:
#line 403 "C.y" /* yacc.c:1646  */
    {
       if(!actionTaken){
          yywarn("Statement does nothing!");
          (yyval.snippet) = new BPatch_nullExpr();
       }else{
          (yyval.snippet) = (yyvsp[-1].snippet);
       } 
       actionTaken = false;
    }
#line 1990 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 18:
#line 414 "C.y" /* yacc.c:1646  */
    {
       if(verbose) printf(" if () ");
       (yyval.snippet) = new BPatch_ifExpr(*(yyvsp[-2].boolExpr), *(yyvsp[0].snippet));
       delete (yyvsp[-2].boolExpr);
       delete (yyvsp[0].snippet);
    }
#line 2001 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 19:
#line 422 "C.y" /* yacc.c:1646  */
    {
       if(verbose) printf(" if () else ");
       (yyval.snippet) = new BPatch_ifExpr(*(yyvsp[-4].boolExpr), *(yyvsp[-2].snippet), *(yyvsp[0].snippet));
       delete (yyvsp[-4].boolExpr);
       delete (yyvsp[-2].snippet);
       delete (yyvsp[0].snippet);
    }
#line 2013 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 21:
#line 435 "C.y" /* yacc.c:1646  */
    {
        (yyval.snippet) = new BPatch_sequence(*(yyvsp[-1].snippetList));
        delete (yyvsp[-1].snippetList);
     }
#line 2022 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 22:
#line 442 "C.y" /* yacc.c:1646  */
    {
       if(strcmp((yyvsp[-3].sval), "break") == 0){
          if(verbose) printf("break_ ()");
          (yyval.snippet) = new BPatch_breakPointExpr();
       }else{
          char *errString = (char *)calloc(strlen((yyvsp[-3].sval)), sizeof(char));
          sprintf(errString, "%s not found!\n", (yyvsp[-3].sval));
          yyerror(errString);
          (yyval.snippet) = new BPatch_nullExpr();
          free(errString);
       }
       delete (yyvsp[-3].sval);
    }
#line 2040 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 23:
#line 458 "C.y" /* yacc.c:1646  */
    { 
       BPatch_function *func = snippetGen->findFunction((yyvsp[-3].sval), *(yyvsp[-1].snippetList));
       if(func == NULL){
          (yyval.snippet) = new BPatch_nullExpr();
          yyerrorNoTok(snippetGen->getError().c_str());
          break;
       }
       (yyval.snippet) = new BPatch_funcCallExpr(*func, *(yyvsp[-1].snippetList));
    }
#line 2054 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 24:
#line 471 "C.y" /* yacc.c:1646  */
    {
       //No parameters, return an empty vector
       (yyval.snippetList) = new BPatch_Vector<BPatch_snippet *>;
    }
#line 2063 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 25:
#line 477 "C.y" /* yacc.c:1646  */
    {
       (yyval.snippetList) = new BPatch_Vector<BPatch_snippet *>;
       (yyval.snippetList)->push_back((yyvsp[0].snippet));
    }
#line 2072 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 26:
#line 483 "C.y" /* yacc.c:1646  */
    {
       if(verbose) printf(" , ");
       (yyvsp[-2].snippetList)->push_back((yyvsp[0].snippet));
       (yyval.snippetList) = (yyvsp[-2].snippetList);
    }
#line 2082 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 27:
#line 492 "C.y" /* yacc.c:1646  */
    {
       (yyval.boolExpr) = new BPatch_boolExpr(BPatch_eq, BPatch_constExpr(0), BPatch_constExpr(0));
    }
#line 2090 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 28:
#line 496 "C.y" /* yacc.c:1646  */
    {
       (yyval.boolExpr) = new BPatch_boolExpr(BPatch_ne, BPatch_constExpr(0), BPatch_constExpr(0));
    }
#line 2098 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 30:
#line 504 "C.y" /* yacc.c:1646  */
    {
       if(verbose) printf(" < ");
       (yyval.boolExpr) = new BPatch_boolExpr(BPatch_lt, *(yyvsp[-2].snippet), *(yyvsp[0].snippet));
    }
#line 2107 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 31:
#line 509 "C.y" /* yacc.c:1646  */
    {
       if(verbose) printf(" > ");
       (yyval.boolExpr) = new BPatch_boolExpr(BPatch_gt, *(yyvsp[-2].snippet), *(yyvsp[0].snippet));
    }
#line 2116 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 32:
#line 514 "C.y" /* yacc.c:1646  */
    {
       if(verbose) printf(" == ");
       (yyval.boolExpr) = new BPatch_boolExpr(BPatch_eq, *(yyvsp[-2].snippet), *(yyvsp[0].snippet));
    }
#line 2125 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 33:
#line 519 "C.y" /* yacc.c:1646  */
    {
       if(verbose) printf(" <= ");
       (yyval.boolExpr) = new BPatch_boolExpr(BPatch_le, *(yyvsp[-2].snippet), *(yyvsp[0].snippet));
    }
#line 2134 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 34:
#line 524 "C.y" /* yacc.c:1646  */
    {
       if(verbose) printf(" >= ");
       (yyval.boolExpr) = new BPatch_boolExpr(BPatch_ge, *(yyvsp[-2].snippet), *(yyvsp[0].snippet));
    }
#line 2143 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 35:
#line 529 "C.y" /* yacc.c:1646  */
    {
       if(verbose) printf(" != ");
       (yyval.boolExpr) = new BPatch_boolExpr(BPatch_ne, *(yyvsp[-2].snippet), *(yyvsp[0].snippet));
    }
#line 2152 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 36:
#line 534 "C.y" /* yacc.c:1646  */
    {
       if(verbose) printf(" AND ");
       (yyval.boolExpr) = new BPatch_boolExpr(BPatch_and, *(yyvsp[-2].boolExpr), *(yyvsp[0].boolExpr));
    }
#line 2161 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 37:
#line 539 "C.y" /* yacc.c:1646  */
    {       if(verbose) printf(" OR ");
       (yyval.boolExpr) = new BPatch_boolExpr(BPatch_or, *(yyvsp[-2].boolExpr), *(yyvsp[0].boolExpr));
    }
#line 2169 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 38:
#line 547 "C.y" /* yacc.c:1646  */
    {
      YYSTYPE::VariableSpec vSpec = {false,false,false,false,false,false,false,false,""};
      vSpec.isGlobal = true;
      (yyval.varSpec) = vSpec;
   }
#line 2179 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 39:
#line 553 "C.y" /* yacc.c:1646  */
    {
      YYSTYPE::VariableSpec vSpec = {false,false,false,false,false,false,false,false,""};
      vSpec.isLocal = true;
      (yyval.varSpec) = vSpec;
   }
#line 2189 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 40:
#line 559 "C.y" /* yacc.c:1646  */
    {
      YYSTYPE::VariableSpec vSpec = {false,false,false,false,false,false,false,false,""};
      vSpec.isParam = true;
      (yyval.varSpec) = vSpec;
   }
#line 2199 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 41:
#line 568 "C.y" /* yacc.c:1646  */
    {
       (yyval.snippet) = snippetGen->findInstVariable(dynC_API::getMangledStub((yyvsp[0].sval), dynCSnippetName).c_str(), (yyvsp[0].sval));
       if((yyval.snippet) == NULL){
          (yyval.snippet) = new BPatch_nullExpr();
          yyerror(snippetGen->getError().c_str());
          break;
       }
    }
#line 2212 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 42:
#line 577 "C.y" /* yacc.c:1646  */
    {
       (yyval.snippet) = snippetGen->findAppVariable((yyvsp[0].sval));
       if((yyval.snippet) == NULL){
          (yyval.snippet) = new BPatch_nullExpr();
          yyerrorNoTokNonUni(snippetGen->getError().c_str());
          break;
       }
    }
#line 2225 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 43:
#line 587 "C.y" /* yacc.c:1646  */
    {
       //disallowed if there is no point specifier
       if(!(yyvsp[-2].varSpec).isGlobal && snippetPoint == NULL){
          yyerrorNoTok("Local variables not allowed when snippet point is unspecified.");
          YYABORT;
          break;
       }
       if((yyvsp[-2].varSpec).isParam){
          (yyval.snippet) = snippetGen->findParameter((yyvsp[0].sval));
       }else{
          (yyval.snippet) = snippetGen->findAppVariable((yyvsp[0].sval), (yyvsp[-2].varSpec).isGlobal, (yyvsp[-2].varSpec).isLocal);
       }
       if((yyval.snippet) == NULL){
          (yyval.snippet) = new BPatch_nullExpr();
          if((yyvsp[-2].varSpec).isGlobal){
             yyerror(snippetGen->getError().c_str());
             YYABORT;
          }else{
             yyerrorNonUni(snippetGen->getError().c_str());
             YYABORT;
          }
          break;
       }
    }
#line 2254 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 44:
#line 613 "C.y" /* yacc.c:1646  */
    {
       //special case for indexed parameters
       if(snippetPoint == NULL){
          yyerrorNoTok("Local variables not allowed when snippet point is unspecified.");
          break;
       }       
       if(!(yyvsp[-2].varSpec).isParam){
          yyerror("Numbered indexes for parameters only");
          (yyval.snippet) = new BPatch_nullExpr();
          break;
       }
       (yyval.snippet) = snippetGen->findParameter((yyvsp[0].ival));
       if((yyval.snippet) == NULL){
          (yyval.snippet) = new BPatch_nullExpr();
          yyerrorNoTokNonUni(snippetGen->getError().c_str());
          break;
       }
    }
#line 2277 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 45:
#line 633 "C.y" /* yacc.c:1646  */
    {
       //array referance
       //check for integer in arith_expression
       (yyval.snippet) = snippetGen->generateArrayRef((yyvsp[-3].snippet), (yyvsp[-1].snippet));
       if((yyval.snippet) == NULL){
          (yyval.snippet) = new BPatch_nullExpr();
          yyerror(snippetGen->getError().c_str());
          break;
       }
    }
#line 2292 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 46:
#line 644 "C.y" /* yacc.c:1646  */
    {
       (yyval.snippet) = (BPatch_snippet *)(new BPatch_arithExpr(BPatch_deref, *(yyvsp[0].snippet)));
    }
#line 2300 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 47:
#line 648 "C.y" /* yacc.c:1646  */
    {
       (yyval.snippet) = (BPatch_snippet *)(new BPatch_arithExpr(BPatch_addr, *(yyvsp[0].snippet)));
    }
#line 2308 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 48:
#line 655 "C.y" /* yacc.c:1646  */
    { 
      if(verbose) printf(" %d ", (yyvsp[0].ival));
      BPatch_snippet * c = new BPatch_constExpr((yyvsp[0].ival));
      (yyval.snippetStringPair) = new std::pair<BPatch_snippet *, const char *>(c, "int");
   }
#line 2318 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 49:
#line 661 "C.y" /* yacc.c:1646  */
    { 
       if(verbose) printf(" %s ", (yyvsp[0].sval));
       BPatch_snippet * c = new BPatch_constExpr((yyvsp[0].sval));
       (yyval.snippetStringPair) = new std::pair<BPatch_snippet *, const char *>(c, "char *");
    }
#line 2328 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 50:
#line 670 "C.y" /* yacc.c:1646  */
    {
        std::vector<std::pair<BPatch_snippet *, const char *> > *cnlist = new std::vector<std::pair<BPatch_snippet *, const char *> >();
        cnlist->push_back(*(yyvsp[0].snippetStringPair));
        (yyval.snippetStringListPair) = cnlist;
     }
#line 2338 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 51:
#line 676 "C.y" /* yacc.c:1646  */
    {
        (yyvsp[-2].snippetStringListPair)->push_back(*(yyvsp[0].snippetStringPair));
        (yyval.snippetStringListPair) = (yyvsp[-2].snippetStringListPair);
     }
#line 2347 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 53:
#line 682 "C.y" /* yacc.c:1646  */
    {(yyval.snippet) = (yyvsp[0].snippetStringPair)->first;}
#line 2353 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 54:
#line 683 "C.y" /* yacc.c:1646  */
    {(yyval.snippet) = new BPatch_nullExpr();}
#line 2359 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 55:
#line 685 "C.y" /* yacc.c:1646  */
    {
   (yyval.snippet) = snippetGen->findRegister((yyvsp[0].sval));
   if ((yyval.snippet) == NULL){
      (yyval.snippet) = new BPatch_nullExpr();
      yyerror(snippetGen->getError().c_str());
   }
}
#line 2371 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 56:
#line 694 "C.y" /* yacc.c:1646  */
    {
       if(verbose) printf("dyninst`%s ", (yyvsp[0].sval));
       
       std::vector<BPatch_snippet *> argVect;
       //snippets w/ return vals
       if(strcmp((yyvsp[0].sval), "function_name") == 0){
          (yyval.snippet) = snippetGen->getContextInfo(SnippetGenerator::SG_FunctionName);
          if((yyval.snippet) == NULL){
             (yyval.snippet) = new BPatch_nullExpr();
             yyerror(snippetGen->getError().c_str());
          }
          break;
       }
       if(strcmp((yyvsp[0].sval), "module_name") == 0){
          (yyval.snippet) = snippetGen->getContextInfo(SnippetGenerator::SG_ModuleName);
          if((yyval.snippet) == NULL){
             (yyval.snippet) = new BPatch_nullExpr();
             yyerror(snippetGen->getError().c_str());
          }
          break;
       }
       if(strcmp((yyvsp[0].sval), "bytes_accessed") == 0){
          (yyval.snippet) = new BPatch_bytesAccessedExpr();
          break;
       }
       if(strcmp((yyvsp[0].sval), "effective_address") == 0){
          (yyval.snippet) = new BPatch_effectiveAddressExpr(); 
          break;
       }
       if(strcmp((yyvsp[0].sval), "original_address") == 0){
          (yyval.snippet) = new BPatch_originalAddressExpr();
          
          break;
       }
       if(strcmp((yyvsp[0].sval), "actual_address") == 0){
          (yyval.snippet) = new BPatch_actualAddressExpr();
          break;
       }
       if(strcmp((yyvsp[0].sval), "return_value") == 0){
          if(snippetGen->getPoint()->getPointType() != BPatch_exit){
             (yyval.snippet) = new BPatch_nullExpr();
             yyerrorNoTokNonUni("Return values only valid at function exit points");
             break;
          }
          (yyval.snippet) = new BPatch_retExpr();
          break;
       }
       if(strcmp((yyvsp[0].sval), "thread_index") == 0){
          (yyval.snippet) = new BPatch_threadIndexExpr();
          break;
       }
       if(strcmp((yyvsp[0].sval), "tid") == 0){
          (yyval.snippet) = snippetGen->getContextInfo(SnippetGenerator::SG_TID);
          if((yyval.snippet) == NULL){
             (yyval.snippet) = new BPatch_nullExpr();
             yyerror(snippetGen->getError().c_str());
          }
          break;
       }
       if(strcmp((yyvsp[0].sval), "dynamic_target") == 0){
          (yyval.snippet) = new BPatch_dynamicTargetExpr();
          break;
       }
 
       yyerror("Syntax error: unrecognized dyninst call");
       (yyval.snippet) = new BPatch_nullExpr();
    }
#line 2443 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 57:
#line 762 "C.y" /* yacc.c:1646  */
    {
       if(verbose) printf(" * ");
       (yyval.snippet) = new BPatch_arithExpr(BPatch_times, *(yyvsp[-2].snippet), *(yyvsp[0].snippet));
       actionTaken = true;
    }
#line 2453 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 58:
#line 768 "C.y" /* yacc.c:1646  */
    {
       (yyval.snippet) = (yyvsp[0].snippet);
       actionTaken = true;
    }
#line 2462 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 59:
#line 773 "C.y" /* yacc.c:1646  */
    {
       if(verbose) printf(" = ");
	    (yyval.snippet) = new BPatch_arithExpr(BPatch_assign, *(yyvsp[-2].snippet), *(yyvsp[0].snippet));
       actionTaken = true;
    }
#line 2472 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 60:
#line 779 "C.y" /* yacc.c:1646  */
    {
       if(verbose) printf(" += ");
       (yyval.snippet) = new BPatch_arithExpr(BPatch_assign, *(yyvsp[-2].snippet), BPatch_arithExpr(BPatch_plus, *(yyvsp[-2].snippet), *(yyvsp[0].snippet)));
       actionTaken = true;
    }
#line 2482 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 61:
#line 785 "C.y" /* yacc.c:1646  */
    {
       if(verbose) printf(" -= ");
       (yyval.snippet) = new BPatch_arithExpr(BPatch_assign, *(yyvsp[-2].snippet), BPatch_arithExpr(BPatch_minus, *(yyvsp[-2].snippet), *(yyvsp[0].snippet)));
       actionTaken = true;
    }
#line 2492 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 62:
#line 791 "C.y" /* yacc.c:1646  */
    {
       if(verbose) printf(" *= ");
       (yyval.snippet) = new BPatch_arithExpr(BPatch_assign, *(yyvsp[-2].snippet), BPatch_arithExpr(BPatch_times, *(yyvsp[-2].snippet), *(yyvsp[0].snippet)));
       actionTaken = true;
    }
#line 2502 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 63:
#line 797 "C.y" /* yacc.c:1646  */
    {
       if(verbose) printf(" /= ");
       (yyval.snippet) = new BPatch_arithExpr(BPatch_assign, *(yyvsp[-2].snippet), BPatch_arithExpr(BPatch_divide, *(yyvsp[-2].snippet), *(yyvsp[0].snippet)));
       actionTaken = true;
    }
#line 2512 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 64:
#line 803 "C.y" /* yacc.c:1646  */
    {
       if(verbose) printf(" %%= ");
       (yyval.snippet) = new BPatch_arithExpr(BPatch_assign, *(yyvsp[-2].snippet), BPatch_arithExpr(BPatch_minus, *(yyvsp[-2].snippet), BPatch_arithExpr(BPatch_times, BPatch_arithExpr(BPatch_divide, *(yyvsp[-2].snippet), *(yyvsp[0].snippet)), *(yyvsp[0].snippet))));
       actionTaken = true;
    }
#line 2522 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 65:
#line 809 "C.y" /* yacc.c:1646  */
    {
       if(verbose) printf(" / ");
       (yyval.snippet) = new BPatch_arithExpr(BPatch_divide, *(yyvsp[-2].snippet), *(yyvsp[0].snippet));
       actionTaken = true;
    }
#line 2532 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 66:
#line 815 "C.y" /* yacc.c:1646  */
    {
       if(verbose) printf(" %% ");
       (yyval.snippet) = new BPatch_arithExpr(BPatch_minus, *(yyvsp[-2].snippet), BPatch_arithExpr(BPatch_times, BPatch_arithExpr(BPatch_divide, *(yyvsp[-2].snippet), *(yyvsp[0].snippet)), *(yyvsp[0].snippet)));
       actionTaken = true;
    }
#line 2542 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 67:
#line 821 "C.y" /* yacc.c:1646  */
    {
       if(verbose) printf(" + ");
       (yyval.snippet) = new BPatch_arithExpr(BPatch_plus, *(yyvsp[-2].snippet), *(yyvsp[0].snippet));
       actionTaken = true;
    }
#line 2552 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 68:
#line 827 "C.y" /* yacc.c:1646  */
    {
       if(verbose) printf(" - ");
       (yyval.snippet) = new BPatch_arithExpr(BPatch_minus, *(yyvsp[-2].snippet), *(yyvsp[0].snippet));
       actionTaken = true;
    }
#line 2562 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 69:
#line 833 "C.y" /* yacc.c:1646  */
    {
       if(dynamic_cast<BPatch_nullExpr *>((yyvsp[-2].snippet))){
          printf("Picked second\n");
          (yyval.snippet) = (yyvsp[0].snippet);
       }else{
          (yyval.snippet) = (yyvsp[-2].snippet);
       }
    }
#line 2575 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 70:
#line 841 "C.y" /* yacc.c:1646  */
    {(yyval.snippet) = (yyvsp[-1].snippet);}
#line 2581 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 71:
#line 843 "C.y" /* yacc.c:1646  */
    {
       (yyval.snippet) = (yyvsp[0].snippet);
       actionTaken = true;
    }
#line 2590 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 72:
#line 852 "C.y" /* yacc.c:1646  */
    {
       if(verbose) printf(" ++ ");
       BPatch_arithExpr addOne = BPatch_arithExpr(BPatch_assign, *(yyvsp[-1].snippet), BPatch_arithExpr(BPatch_plus, *(yyvsp[-1].snippet), BPatch_constExpr(1)));
       (yyval.snippet) = new BPatch_arithExpr(BPatch_seq, (BPatch_snippet &)addOne, BPatch_arithExpr(BPatch_minus, *(yyvsp[-1].snippet), BPatch_constExpr(1)));
    }
#line 2600 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 73:
#line 858 "C.y" /* yacc.c:1646  */
    {
       if(verbose) printf(" ++ ");
       BPatch_arithExpr addOne = BPatch_arithExpr(BPatch_assign, *(yyvsp[0].snippet), BPatch_arithExpr(BPatch_plus, *(yyvsp[0].snippet), BPatch_constExpr(1)));
       (yyval.snippet) = new BPatch_arithExpr(BPatch_seq, (BPatch_snippet &)addOne, *(yyvsp[0].snippet));
    }
#line 2610 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 74:
#line 864 "C.y" /* yacc.c:1646  */
    {
       if(verbose) printf(" -- ");
       BPatch_arithExpr subOne = BPatch_arithExpr(BPatch_assign, *(yyvsp[-1].snippet), BPatch_arithExpr(BPatch_minus, *(yyvsp[-1].snippet), BPatch_constExpr(1)));
       (yyval.snippet) = new BPatch_arithExpr(BPatch_seq, (BPatch_snippet &)subOne, BPatch_arithExpr(BPatch_plus, *(yyvsp[-1].snippet), BPatch_constExpr(1)));
    }
#line 2620 "dynC.tab.C" /* yacc.c:1646  */
    break;

  case 75:
#line 870 "C.y" /* yacc.c:1646  */
    {
       if(verbose) printf(" -- ");
       BPatch_arithExpr subOne = BPatch_arithExpr(BPatch_assign, *(yyvsp[0].snippet), BPatch_arithExpr(BPatch_minus, *(yyvsp[0].snippet), BPatch_constExpr(1)));
       (yyval.snippet) = new BPatch_arithExpr(BPatch_seq, (BPatch_snippet &)subOne, *(yyvsp[0].snippet));
    }
#line 2630 "dynC.tab.C" /* yacc.c:1646  */
    break;


#line 2634 "dynC.tab.C" /* yacc.c:1646  */
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;
  *++yylsp = yyloc;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }

  yyerror_range[1] = yylloc;

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

  yyerror_range[1] = yylsp[1-yylen];
  /* Do not reclaim the symbols of the rule whose action triggered
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
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
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

      yyerror_range[1] = *yylsp;
      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp, yylsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  yyerror_range[2] = yylloc;
  /* Using YYLLOC is tempting, but would change the location of
     the lookahead.  YYLOC is available though.  */
  YYLLOC_DEFAULT (yyloc, yyerror_range, 2);
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

#if !defined yyoverflow || YYERROR_VERBOSE
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
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, &yylloc);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
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
  return yyresult;
}
#line 877 "C.y" /* yacc.c:1906  */


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
