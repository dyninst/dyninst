%{
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

%}


%union {
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
};

%locations

%nonassoc KNOWN_ERROR_TOK

%token <sval> IDENTIFIER CONSTANT STRING TYPE
%token <ival> NUMBER
%token <context> ERROR
%token EOL
%token SIZEOF TRUE FALSE
%token PTR_OP INC_OP DEC_OP LEFT_OP RIGHT_OP 
%token LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN
%token XOR_ASSIGN OR_ASSIGN TYPE_NAME

%token TYPEDEF EXTERN STATIC
%token CHAR SHORT INT LONG SIGNED UNSIGNED FLOAT DOUBLE CONST VOID
%token STRUCT UNION ENUM ELLIPSIS 
%token IF
%token LOCAL PARAM GLOBAL INF DYNINST INST
%token NEWLINE

%token CASE DEFAULT SWITCH RETURN
%token NILL


%left DOT ASTERISK AMPERSAND COMMA
%left NOT
%left OR
%left AND
%left LESS_EQ GREATER_EQ EQ NOT_EQ
%left '+' '-'
%left '*' '/' '%'
%left '|'
%left COLON
%left SEMI
%left START_BLOCK END_BLOCK
%left DOLLAR
%left BACKTICK

%right '!' '~' '&'
%right AND_OP OR_OP MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN SUB_ASSIGN ASSIGN

%nonassoc NOPEN NCLOSE
%nonassoc LOWER_THAN_ELSE //used to trick Bison into accepting if else statements
%nonassoc ELSE
%nonassoc LOWER_THAN_DEREF

%type <boolExpr> bool_expression bool_constant
%type <snippet> arith_expression variable_expr statement inc_decr_expr func_call  block
%type <snippet> var_declaration
%type <snippetList> param_list statement_list
%type <snippetStringListPair> const_list
%type <snippetStringPair> constant
%type <varSpec> var_decl_modifiers var_modifiers

%%

start:
    statement_list
    { 
       oneTimeCount = 0;
       oneTimeGblCount = 0;
       $1->insert($1->end(), endSnippets.begin(), endSnippets.end());
       parse_result = new BPatch_sequence(*$1); 
       delete $1;
       endSnippets.clear();
       if(verbose) {
          printf("\n");
          fflush(stdout);
       }
    }
    ; 

var_declaration: var_decl_modifiers IDENTIFIER 
    {
       //IDENTIFIER leaks, but how to fix b/c use of $0?
       std::string mangledName;
       mangledName = dynC_API::mangle($2, dynCSnippetName, $1.type);
       if(verbose) printf("name : %s\n", mangledName.c_str());
       $$ = snippetGen->findOrCreateVariable(mangledName.c_str(), $1.type);
       if($$ == NULL){
          $$ = new BPatch_nullExpr();
          char *errString = strdup(snippetGen->getError().c_str());
          yyerror(errString);
          free(errString);
          break;
       }

        if(!($1.isStatic || $1.isGlobal)){ 
          BPatch_snippet setSn = BPatch_constExpr(0); // if int etc.
          if(strcmp($1.type, "char *") == 0){
             setSn = BPatch_constExpr("");
          }
          endSnippets.push_back(new BPatch_arithExpr(BPatch_assign, *$$, setSn));
          }
    }
    | var_decl_modifiers IDENTIFIER ASSIGN arith_expression
    {   
      
       std::string mangledName = dynC_API::mangle($2, dynCSnippetName, $1.type);
       if(verbose) printf("name : %s\n", mangledName.c_str());
       BPatch_snippet *alloc = snippetGen->findOrCreateVariable(mangledName.c_str(), $1.type);
       if(alloc == NULL){
          $$ = new BPatch_nullExpr();
          char *errString = strdup(snippetGen->getError().c_str());
          yyerror(errString);
          free(errString);
          break;
       }
       BPatch_arithExpr *assign = new BPatch_arithExpr(BPatch_assign, *alloc, *$4);
       $$ = assign;

       if($1.isStatic || $1.isGlobal){
          makeOneTimeStatementGbl(*$$);
       }       
    }
    | var_decl_modifiers IDENTIFIER '[' NUMBER ']' 
    {
       //IDENTIFIER leaks, but how to fix b/c use of $0?
       if($4 < 0){
          std::stringstream errMessage;
          errMessage << "Invalid array size: " << $4;
          char *eM = strdup(errMessage.str().c_str());
          yyerrorNoTok(eM);
          free(eM);
          $$ = new BPatch_nullExpr();
          break;
       }
      
       std::stringstream type;
       type << $1.type << "[" << $4 << "]";
       std::string mangledName = dynC_API::mangle($2, dynCSnippetName, type.str().c_str());
      
       $$ = snippetGen->findOrCreateArray(mangledName.c_str(), $1.type, $4);
       if($$ == NULL){
          $$ = new BPatch_nullExpr();
          char *errString = strdup(snippetGen->getError().c_str());
          yyerror(errString);
          free(errString);
          break;
       }
       if(!($1.isStatic || $1.isGlobal)){ 
          BPatch_snippet setSn = BPatch_constExpr(0); // if int etc.
          if(strcmp($1.type, "char *") == 0){
             setSn = BPatch_constExpr("");
          }
          for(int n = 0; n < $4; ++n){
             endSnippets.push_back(new BPatch_arithExpr(BPatch_assign, BPatch_arithExpr(BPatch_ref, *$$, BPatch_constExpr(n)), setSn));             
          }
       }

    }
    | var_decl_modifiers IDENTIFIER '[' ']' ASSIGN '{' const_list '}'
    {
       //IDENTIFIER leaks, but how to fix b/c use of $0?
       std::stringstream type;
       type << $1.type << "[" << $7->size() << "]";
       std::string mangledName = dynC_API::mangle($2, dynCSnippetName, type.str().c_str());

       $$ = snippetGen->findOrCreateArray(mangledName.c_str(), $1.type, $7->size()); //will only allocate memory if nessessary
       if($$ == NULL){
          $$ = new BPatch_nullExpr();
          char *errString = strdup(snippetGen->getError().c_str());
          yyerror(errString);
          free(errString);
          break;
       }
       std::vector<BPatch_snippet* > *assignVect = new std::vector<BPatch_snippet *>();
       assignVect->push_back($$);
       for(unsigned int n = 0; n < $7->size(); ++n){
          if(strcmp($1.type, (*$7)[n].second) != 0){
             std::string errMessage = "Type conflict when trying to initialize array of type \'";
             errMessage += type.str();
             errMessage += "\' with a value of type ";
             errMessage += (*$7)[n].second;
             char *eM = strdup(errMessage.c_str());
             yyerrorNoTok(eM);
             free(eM);
             $$ = new BPatch_nullExpr();
             break;
          }
          BPatch_snippet *assign = new BPatch_arithExpr(BPatch_assign, BPatch_arithExpr(BPatch_ref, *$$, BPatch_constExpr(n)), *(*$7)[n].first);      
          
          if($1.isStatic || $1.isGlobal){
              makeOneTimeStatement(*assign);
              }
          //makeOneTimeStatement(*assign);
          assignVect->push_back(assign);
       }
       $$ = new BPatch_sequence(*assignVect);
    }
    | var_decl_modifiers IDENTIFIER '[' NUMBER ']' ASSIGN '{' const_list '}'
    {
       //IDENTIFIER leaks, but how to fix b/c use of $0?
       std::vector<BPatch_snippet *> argVect;
       std::stringstream type;
       if($4 < 0){
          std::stringstream errMessage;
          errMessage << "Invalid array size: " << $4;
          char *eM = strdup(errMessage.str().c_str());
          yyerrorNoTok(eM);
          free(eM);
          $$ = new BPatch_nullExpr();
          break;
       }
       type << $1.type << "[" << $4 << "]";
       std::string mangledName = dynC_API::mangle($2, dynCSnippetName, type.str().c_str());

       $$ = snippetGen->findOrCreateArray(mangledName.c_str(), $1.type, $4); //will only allocate memory if nessessary
       if($$ == NULL){
          $$ = new BPatch_nullExpr();
          char *errString = strdup(snippetGen->getError().c_str());
          yyerror(errString);
          free(errString);
          break;
       }
       std::vector<BPatch_snippet* > *assignVect = new std::vector<BPatch_snippet *>();
       assignVect->push_back($$);
       if((unsigned int)$4 != $8->size()){
          yyerrorNoTok("Invalid number of arguments given in array initialization");
          $$ = new BPatch_nullExpr();
          break;
       }
       for(int n = 0; n < $4; ++n){
          if(strcmp($1.type, (*$8)[n].second) != 0){
             std::string errMessage = "Type conflict when trying to initialize array of type \'";
             errMessage += type.str();
             errMessage += "\' with a value of type ";
             errMessage += (*$8)[n].second;
             char *eM = strdup(errMessage.c_str());
             yyerrorNoTok(eM);
             free(eM);
             $$ = new BPatch_nullExpr();
             break;
          }
          BPatch_snippet *assign = new BPatch_arithExpr(BPatch_assign, BPatch_arithExpr(BPatch_ref, *$$, BPatch_constExpr(n)), *(*$8)[n].first);      
          if($1.isStatic || $1.isGlobal){
             makeOneTimeStatement(*assign);
          }
          //   makeOneTimeStatement(*assign);
          assignVect->push_back(assign);
       
       }
       $$ = new BPatch_sequence(*assignVect);
    }
;

var_decl_modifiers: TYPE 
   {
      YYSTYPE::VariableSpec rSpec = {false,false,false,false,false,false,false,false,$1};
      $$ = rSpec;
   } 
   | STATIC var_decl_modifiers
   {
      if ($2.isStatic){
         //throw error: two static
         yyerror("Syntax error");
      }else{
         $2.isStatic = true;
      }
      $$ = $2;
  }
/*   | CONST var_decl_modifiers
   {
      if ($2.isConstant){
         //throw error: two consts
         yyerror("Syntax error");
      }else{
         $2.isConstant = true;
      }
      $$ = $2;
   }
   | GLOBAL var_decl_modifiers
   {
      if ($2.isGlobal){
         //throw error: two globals
         yyerror("Syntax error");
      }else if($2.isLocal){
         //throw error: can't be global and local
         yyerror("Syntax error: variable cannot be both local and global");
      }else {
         $2.isGlobal = true;
      }
      $$ = $2;   
   }
   | LOCAL var_decl_modifiers
   {
      if ($2.isLocal){
         //throw error: two locals
         yyerror("Syntax error");
      }else if($2.isGlobal){
         yyerror("Syntax error: variable cannot be both local and global");
         //throw error: can't be global and local
      }else{
         $2.isLocal = true;
      }
      $$ = $2;   
   }
   | THREAD var_decl_modifiers
   {
      if ($2.isThread){
         //throw error: two thread
         yyerror("Syntax error");
      }else{
         yyerror("Thread variables not supported");
         $2.isThread = true;
      }
      $$ = $2;   
   }
   | MACHINE var_decl_modifiers
   {
      if ($2.isMachineState){
         //error
         yyerror("Syntax error");
      }else{
         yyerror("Machine state variables not supported");
         $2.isMachineState = true;
      }
      $$ = $2;   
      }*/
   ;


statement_list:
    statement
    { 
       if(verbose) printf("\n");
       $$ = new BPatch_Vector<BPatch_snippet *>; 
       $$->push_back($1);
    }
    | statement_list statement
    {
       $1->push_back($2);
       $$ = $1;
    }
    | NOPEN statement_list NCLOSE
    {
       BPatch_sequence *seq = new BPatch_sequence(*$2);
       makeOneTimeStatementGbl(*seq);
       std::vector<BPatch_snippet *> *retVect = new std::vector<BPatch_snippet *>;
       retVect->push_back(seq);
       $$ = retVect;       
    }
    | statement_list NOPEN statement_list NCLOSE
    {
       BPatch_sequence seq = BPatch_sequence(*$3);
       makeOneTimeStatementGbl(seq);
       $1->push_back(&seq);
       $$ = $1;
    }
    ;

statement:
    error // built in error recovery point
    {
       $$ = new BPatch_nullExpr();
       actionTaken = false;
    }
    | ERROR // lex error token
    {
       yyerrorNoTok($1);
       $$ = new BPatch_nullExpr();
       actionTaken = false;
    }
    | var_declaration SEMI
    {
       $$ = $1;
    } 
    // 1 + 2;
    | arith_expression SEMI
    {
       if(!actionTaken){
          yywarn("Statement does nothing!");
          $$ = new BPatch_nullExpr();
       }else{
          $$ = $1;
       } 
       actionTaken = false;
    }
  // if (x == y) {}
    | IF '(' bool_expression ')' block %prec LOWER_THAN_ELSE //used to trick Bison into accepting IF ELSE w/out shift/reduce error
    {
       if(verbose) printf(" if () ");
       $$ = new BPatch_ifExpr(*$3, *$5);
       delete $3;
       delete $5;
    }
    // if (x == y) {} else {}
    | IF '(' bool_expression ')' block ELSE block 
    {
       if(verbose) printf(" if () else ");
       $$ = new BPatch_ifExpr(*$3, *$5, *$7);
       delete $3;
       delete $5;
       delete $7;
    }
    //////////////// Error Cases /////////////////////
    ;

block: statement
     // {stuff}
     | '{' statement_list '}'
     {
        $$ = new BPatch_sequence(*$2);
        delete $2;
     }
     ;

func_call: DYNINST BACKTICK IDENTIFIER '(' param_list ')'
    {
       //TODO: built in dyninst actions and (future) other snippet calls?
       if(strcmp($3, "break") == 0){
          if(verbose) printf("break_ ()");
          $$ = new BPatch_breakPointExpr();
       }else if(strcmp($3, "stopThread") == 0){
          //how?
          if(verbose) printf("stopThread ()");
          $$ = new BPatch_nullExpr();
       }else{
          yyerror("Dyninst function not found!\n");
          $$ = new BPatch_nullExpr();
       }
       delete $3;
    }
   
    |INF BACKTICK IDENTIFIER '(' param_list ')'
           //funct(stuff,stuff)
    { 
       BPatch_function *func = snippetGen->findFunction($3, *$5);
       if(func == NULL){
          $$ = new BPatch_nullExpr();
          char *errString = strdup(snippetGen->getError().c_str());
          yyerrorNoTok(errString);
          free(errString);
          break;
       }
       if($5 == NULL){
          fprintf(stderr, "Internal Error: Param_list is null!\n");
       }
       $$ = new BPatch_funcCallExpr(*func, *$5);
    }
    ;

param_list: 
    //()
    {
       //No parameters, return an empty vector
       $$ = new BPatch_Vector<BPatch_snippet *>;
    }
    //(stuff)
    | arith_expression
    { 
       $$ = new BPatch_Vector<BPatch_snippet *>; 
       $$->push_back($1);
    }
    //(stuff,stuff)
    | param_list COMMA arith_expression
    { 
       if(verbose) printf(" , ");
       $1->push_back($3); 
       $$ = $1;
    }
    ;


bool_constant: TRUE 
    { 
       $$ = new BPatch_boolExpr(BPatch_eq, BPatch_constExpr(0), BPatch_constExpr(0));
    }
    | FALSE
    {
       $$ = new BPatch_boolExpr(BPatch_ne, BPatch_constExpr(0), BPatch_constExpr(0));
    }
    ;


bool_expression: bool_constant 
    |arith_expression '<' arith_expression
    {
       if(verbose) printf(" < ");
       $$ = new BPatch_boolExpr(BPatch_lt, *$1, *$3);
    }
    |	arith_expression '>' arith_expression
    {
       if(verbose) printf(" > ");
       $$ = new BPatch_boolExpr(BPatch_gt, *$1, *$3);
    }
    | arith_expression EQ arith_expression
    {
       if(verbose) printf(" == ");
       $$ = new BPatch_boolExpr(BPatch_eq, *$1, *$3);
    }
    | arith_expression LESS_EQ arith_expression
    {
       if(verbose) printf(" <= ");
       $$ = new BPatch_boolExpr(BPatch_le, *$1, *$3);
    }
    | arith_expression GREATER_EQ arith_expression 
    {
       if(verbose) printf(" >= ");
       $$ = new BPatch_boolExpr(BPatch_ge, *$1, *$3);
    }
    | arith_expression NOT_EQ arith_expression 
    {
       if(verbose) printf(" != ");
       $$ = new BPatch_boolExpr(BPatch_ne, *$1, *$3);
    }
    |	bool_expression AND bool_expression  
    {
       if(verbose) printf(" AND ");
       $$ = new BPatch_boolExpr(BPatch_and, *$1, *$3);
    }
    | bool_expression OR bool_expression  
    {       if(verbose) printf(" OR ");
       $$ = new BPatch_boolExpr(BPatch_or, *$1, *$3);
    }

    // add not
    ;


var_modifiers:
   GLOBAL
   {
      YYSTYPE::VariableSpec vSpec = {false,false,false,false,false,false,false,false,""};
      vSpec.isGlobal = true;
      $$ = vSpec;   
   }
   | LOCAL
   {
      YYSTYPE::VariableSpec vSpec = {false,false,false,false,false,false,false,false,""};
      vSpec.isLocal = true;
      $$ = vSpec;   
   }
   | PARAM
   {
      YYSTYPE::VariableSpec vSpec = {false,false,false,false,false,false,false,false,""};
      vSpec.isParam = true;
      $$ = vSpec;   
   }
/*   | THREAD
   {
     YYSTYPE::VariableSpec vSpec = {false,false,false,false,false,false,false,false,""};
      vSpec.isThread = true;
      $$ = vSpec;   
   }
   | MACHINE
   {
      YYSTYPE::VariableSpec vSpec = {false,false,false,false,false,false,false,false,""};
      vSpec.isMachineState = true;
      $$ = vSpec;   
      }*/
   ;


variable_expr: IDENTIFIER
    {
       $$ = snippetGen->findInstVariable(dynC_API::getMangledStub($1, dynCSnippetName).c_str(), $1);
       if($$ == NULL){
          $$ = new BPatch_nullExpr();
          char *errString = strdup(snippetGen->getError().c_str());
          yyerror(errString);
          free(errString);
          break;
       }
    }
    | '*' IDENTIFIER
    {
       $$ = snippetGen->findInstVariable(dynC_API::getMangledStub($2, dynCSnippetName).c_str(), $2);
       if($$ == NULL){
          $$ = new BPatch_nullExpr();
          char *errString = strdup(snippetGen->getError().c_str());
          yyerror(errString);
          free(errString);
          break;
       }
       $$ = (BPatch_snippet *)(new BPatch_arithExpr(BPatch_deref, *$$));       
    }
    | '&' IDENTIFIER
    {
       $$ = snippetGen->findInstVariable(dynC_API::getMangledStub($2, dynCSnippetName).c_str(), $2);
       if($$ == NULL){
          $$ = new BPatch_nullExpr();
          char *errString = strdup(snippetGen->getError().c_str());
          yyerror(errString);
          free(errString);
          break;
       }
       $$ = (BPatch_snippet *)(new BPatch_arithExpr(BPatch_addr, *$$));       
    }

    | INF BACKTICK IDENTIFIER
    {
       $$ = snippetGen->findAppVariable($3);
       if($$ == NULL){
          $$ = new BPatch_nullExpr();
          char *errString = strdup(snippetGen->getError().c_str());
          yyerrorNoTokNonUni(errString);
          free(errString);
          break;
       }
    }
    | '*' INF BACKTICK IDENTIFIER
    {
       $$ = snippetGen->findAppVariable($4);
       if($$ == NULL){
          $$ = new BPatch_nullExpr();
          char *errString = strdup(snippetGen->getError().c_str());
          yyerrorNoTokNonUni(errString);
          free(errString);
          break;
       }
       $$ = (BPatch_snippet *)(new BPatch_arithExpr(BPatch_deref, *$$));       
    }
    | '&' INF BACKTICK IDENTIFIER
    {
       $$ = snippetGen->findAppVariable($4);
       if($$ == NULL){
          $$ = new BPatch_nullExpr();
          char *errString = strdup(snippetGen->getError().c_str());
          yyerrorNoTokNonUni(errString);
          free(errString);
          break;
       }
       $$ = (BPatch_snippet *)(new BPatch_arithExpr(BPatch_addr, *$$));       
    }


    | var_modifiers BACKTICK IDENTIFIER
    {
       //disallowed if there is no point specifier
       if(!$1.isGlobal && snippetPoint == NULL){
          yyerrorNoTok("Local variables not allowed when snippet point is unspecified.");
          YYABORT;
          break;
       }
       if($1.isParam){
          $$ = snippetGen->findParameter($3);
       }else{
          $$ = snippetGen->findAppVariable($3, $1.isGlobal, $1.isLocal);
       }
       if($$ == NULL){
          $$ = new BPatch_nullExpr();
          if($1.isGlobal){
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
    }
    | '*' var_modifiers BACKTICK IDENTIFIER
    {
       if(snippetPoint == NULL){
          yyerrorNoTok("Local variables not allowed when snippet point is unspecified.");
          break;
       }       
       if($2.isParam){
          $$ = snippetGen->findParameter($4);
       }else{
          $$ = snippetGen->findAppVariable($4, $2.isGlobal, $2.isLocal);
       }
       if($$ == NULL){
          $$ = new BPatch_nullExpr();
          if($2.isGlobal){
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
       $$ = (BPatch_snippet *)(new BPatch_arithExpr(BPatch_deref, *$$));       
    }

    | '&' var_modifiers BACKTICK IDENTIFIER
    {
       if(snippetPoint == NULL){
          yyerrorNoTok("Local variables not allowed when snippet point is unspecified.");
          break;
       }       
       if($2.isParam){
          $$ = snippetGen->findParameter($4);
       }else{
          $$ = snippetGen->findAppVariable($4, $2.isGlobal, $2.isLocal);
       }
       if($$ == NULL){
          $$ = new BPatch_nullExpr();
          if($2.isGlobal){
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
       $$ = (BPatch_snippet *)(new BPatch_arithExpr(BPatch_addr, *$$));       
    }

    | var_modifiers BACKTICK NUMBER 
    {
       //special case for indexed parameters
       if(snippetPoint == NULL){
          yyerrorNoTok("Local variables not allowed when snippet point is unspecified.");
          break;
       }       
       if(!$1.isParam){
          yyerror("Numbered indexes for parameters only");
          $$ = new BPatch_nullExpr();
          break;
       }
       $$ = snippetGen->findParameter($3);
       if($$ == NULL){
          $$ = new BPatch_nullExpr();
          char *errString = strdup(snippetGen->getError().c_str());
          yyerrorNoTokNonUni(errString);
          free(errString);
          break;
       }
    }
    | '*' var_modifiers BACKTICK NUMBER 
    {
       //special case for indexed parameters
       if(snippetPoint == NULL){
          yyerrorNoTok("Local variables not allowed when snippet point is unspecified.");
          break;
       }       
       if(!$2.isParam){
          yyerror("Numbered indexes for parameters only");
          $$ = new BPatch_nullExpr();
          break;
       }
       $$ = snippetGen->findParameter($4);
       if($$ == NULL){
          $$ = new BPatch_nullExpr();
          char *errString = strdup(snippetGen->getError().c_str());
          yyerrorNoTokNonUni(errString);
          free(errString);
          break;
       }
       $$ = (BPatch_snippet *)(new BPatch_arithExpr(BPatch_deref, *$$));
    }
    | '&' var_modifiers BACKTICK NUMBER 
    {
       //special case for indexed parameters
       if(snippetPoint == NULL){
          yyerrorNoTok("Local variables not allowed when snippet point is unspecified.");
          break;
       }       

       if(!$2.isParam){
          yyerror("Numbered indexes for parameters only");
          $$ = new BPatch_nullExpr();
          break;
       }
       $$ = snippetGen->findParameter($4);
       if($$ == NULL){
          $$ = new BPatch_nullExpr();
          char *errString = strdup(snippetGen->getError().c_str());
          yyerrorNoTokNonUni(errString);
          free(errString);
          break;
       }
       $$ = (BPatch_snippet *)(new BPatch_arithExpr(BPatch_addr, *$$));
    }

    | variable_expr '[' arith_expression ']'
    {
       //array referance
       //check for integer in arith_expression
       $$ = snippetGen->generateArrayRef($1, $3);
       if($$ == NULL){
          $$ = new BPatch_nullExpr();
          char *errString = strdup(snippetGen->getError().c_str());
          yyerror(errString);
          free(errString);
          break;
       }
    }
/*
    | '*' variable_expr 
    {
       //dereference variable
       printf("DEREF!\n");
       $$ = (BPatch_snippet *)(new BPatch_arithExpr(BPatch_deref, *$2));
    }
    | '&' variable_expr
    {
       //referance variable
       $$ = (BPatch_snippet *)(new BPatch_arithExpr(BPatch_addr, *$2));
       }*/
    ;

constant: 
   NUMBER     
   { 
      if(verbose) printf(" %d ", $1);
      BPatch_snippet * c = new BPatch_constExpr($1);
      $$ = new std::pair<BPatch_snippet *, char *>(c, "int");
   }
    | STRING     
    { 
       if(verbose) printf(" %s ", $1);
       BPatch_snippet * c = new BPatch_constExpr($1);
       $$ = new std::pair<BPatch_snippet *, char *>(c, "char *");
    }
;

const_list: 
     constant
     {
        std::vector<std::pair<BPatch_snippet *, char *> > *cnlist = new std::vector<std::pair<BPatch_snippet *, char *> >();
        cnlist->push_back(*$1);
        $$ = cnlist;
     }
     | const_list COMMA constant
     {
        $1->push_back(*$3);
        $$ = $1;
     }

arith_expression: variable_expr
| constant {$$ = $1->first}
| NILL {$$ = new BPatch_nullExpr();}
   | DYNINST BACKTICK IDENTIFIER
    {
       if(verbose) printf("dyninst`%s ", $3);
       
       std::vector<BPatch_snippet *> argVect;
       //snippets w/ return vals
       if(strcmp($3, "function_name") == 0){
          $$ = snippetGen->getContextInfo(SnippetGenerator::SG_FunctionName);
          if($$ == NULL){
             $$ = new BPatch_nullExpr();
             char *errString = strdup(snippetGen->getError().c_str());
             yyerror(errString);
             free(errString);
          }
          break;
       }
       if(strcmp($3, "module_name") == 0){
          $$ = snippetGen->getContextInfo(SnippetGenerator::SG_ModuleName);
          if($$ == NULL){
             $$ = new BPatch_nullExpr();
             char *errString = strdup(snippetGen->getError().c_str());
             yyerror(errString);
             free(errString);
          }
          break;
       }
       if(strcmp($3, "bytes_accessed") == 0){
          $$ = new BPatch_bytesAccessedExpr();
          break;
       }
       if(strcmp($3, "effective_address") == 0){
          $$ = new BPatch_effectiveAddressExpr(); 
          break;
       }
       if(strcmp($3, "original_address") == 0){
          $$ = new BPatch_originalAddressExpr();
          
          break;
       }
       if(strcmp($3, "actual_address") == 0){
          $$ = new BPatch_actualAddressExpr();
          break;
       }
       if(strcmp($3, "return_value") == 0){
          if(snippetGen->getPoint()->getPointType() != BPatch_exit){
             $$ = new BPatch_nullExpr();
             yyerrorNoTokNonUni("Return values only valid at function exit points");
             break;
          }
          $$ = new BPatch_retExpr();
          break;
       }
       if(strcmp($3, "thread_index") == 0){
          $$ = new BPatch_threadIndexExpr();
          break;
       }
       if(strcmp($3, "tid") == 0){
          $$ = snippetGen->getContextInfo(SnippetGenerator::SG_TID);
          if($$ == NULL){
             $$ = new BPatch_nullExpr();
             char *errString = strdup(snippetGen->getError().c_str());
             yyerror(errString);
             free(errString);
          }
          break;
       }
       if(strcmp($3, "dynamic_target") == 0){
          $$ = new BPatch_dynamicTargetExpr();
          break;
       }
 
       yyerror("Syntax error: unrecognized dyninst call");
       $$ = new BPatch_nullExpr()
    }
    |	arith_expression '*' arith_expression 
    {
       if(verbose) printf(" * ");
       $$ = new BPatch_arithExpr(BPatch_times, *$1, *$3);
       actionTaken = true;
    }
    | func_call 
    {
       $$ = $1;
       actionTaken = true;
    }
    | variable_expr ASSIGN arith_expression
    {
       if(verbose) printf(" = ");
	    $$ = new BPatch_arithExpr(BPatch_assign, *$1, *$3);
       actionTaken = true;
    }
    | variable_expr ADD_ASSIGN arith_expression
    {
       if(verbose) printf(" += ");
       $$ = new BPatch_arithExpr(BPatch_assign, *$1, BPatch_arithExpr(BPatch_plus, *$1, *$3));
       actionTaken = true;
    }
    | variable_expr SUB_ASSIGN arith_expression
    {
       if(verbose) printf(" -= ");
       $$ = new BPatch_arithExpr(BPatch_assign, *$1, BPatch_arithExpr(BPatch_minus, *$1, *$3));
       actionTaken = true;
    }
    | variable_expr MUL_ASSIGN arith_expression
    {
       if(verbose) printf(" *= ");
       $$ = new BPatch_arithExpr(BPatch_assign, *$1, BPatch_arithExpr(BPatch_times, *$1, *$3));
       actionTaken = true;
    }
    | variable_expr DIV_ASSIGN arith_expression
    {
       if(verbose) printf(" /= ");
       $$ = new BPatch_arithExpr(BPatch_assign, *$1, BPatch_arithExpr(BPatch_divide, *$1, *$3));
       actionTaken = true;
    }
    | variable_expr MOD_ASSIGN arith_expression
    {
       if(verbose) printf(" %%= ");
       $$ = new BPatch_arithExpr(BPatch_assign, *$1, BPatch_arithExpr(BPatch_minus, *$1, BPatch_arithExpr(BPatch_times, BPatch_arithExpr(BPatch_divide, *$1, *$3), *$3)));
       actionTaken = true;
    }
    |	arith_expression '/' arith_expression 
    {
       if(verbose) printf(" / ");
       $$ = new BPatch_arithExpr(BPatch_divide, *$1, *$3);
       actionTaken = true;
    }
    | arith_expression '%' arith_expression
    {
       if(verbose) printf(" %% ");
       $$ = new BPatch_arithExpr(BPatch_minus, *$1, BPatch_arithExpr(BPatch_times, BPatch_arithExpr(BPatch_divide, *$1, *$3), *$3));
       actionTaken = true;
    }
    |	arith_expression '+' arith_expression 
    {
       if(verbose) printf(" + ");
       $$ = new BPatch_arithExpr(BPatch_plus, *$1, *$3);
       actionTaken = true;
    }
    |	arith_expression '-' arith_expression 
    {
       if(verbose) printf(" - ");
       $$ = new BPatch_arithExpr(BPatch_minus, *$1, *$3);
       actionTaken = true;
    }
    | arith_expression '|' arith_expression
    {
       if(dynamic_cast<BPatch_nullExpr *>($1)){
          printf("Picked second\n");
          $$ = $3;
       }else{
          $$ = $1;
       }
    }
    |	'(' arith_expression ')'  {$$ = $2;}
    | inc_decr_expr 
    {
       $$ = $1;
       actionTaken = true;
    }     
    ;


inc_decr_expr:
    variable_expr INC_OP 
    {
       if(verbose) printf(" ++ ");
       BPatch_arithExpr addOne = BPatch_arithExpr(BPatch_assign, *$1, BPatch_arithExpr(BPatch_plus, *$1, BPatch_constExpr(1)));
       $$ = new BPatch_arithExpr(BPatch_seq, (BPatch_snippet &)addOne, BPatch_arithExpr(BPatch_minus, *$1, BPatch_constExpr(1)));
    }
    | INC_OP variable_expr 
    {
       if(verbose) printf(" ++ ");
       BPatch_arithExpr addOne = BPatch_arithExpr(BPatch_assign, *$2, BPatch_arithExpr(BPatch_plus, *$2, BPatch_constExpr(1)));
       $$ = new BPatch_arithExpr(BPatch_seq, (BPatch_snippet &)addOne, *$2);
    }
    | variable_expr DEC_OP
    {
       if(verbose) printf(" -- ");
       BPatch_arithExpr subOne = BPatch_arithExpr(BPatch_assign, *$1, BPatch_arithExpr(BPatch_minus, *$1, BPatch_constExpr(1)));
       $$ = new BPatch_arithExpr(BPatch_seq, (BPatch_snippet &)subOne, BPatch_arithExpr(BPatch_plus, *$1, BPatch_constExpr(1)));
    }
    | DEC_OP variable_expr
    {
       if(verbose) printf(" -- ");
       BPatch_arithExpr subOne = BPatch_arithExpr(BPatch_assign, *$2, BPatch_arithExpr(BPatch_minus, *$2, BPatch_constExpr(1)));
       $$ = new BPatch_arithExpr(BPatch_seq, (BPatch_snippet &)subOne, *$2);
    }
    ;

%%

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
