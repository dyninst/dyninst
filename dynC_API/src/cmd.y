//cmd.y
%{ 

/*                                                                              
 * Yacc will define token IF, conflicting with dyn_regs.h                       
 * This undef is safe b/c yacc also creates an enum IF                          
 * which serves the same purpose.                                               
 */
#undef IF

#include <string.h>
#include <map>
#include "BPatch_type.h"
#include "BPatch_snippet.h"
#include "symtab.h"

extern "C" {
      void yyerror(char *s);
      int yyparse(void);
}

int yylex();

extern BPatch_process *appProc;
extern BPatch_image *appImage;

//SymbolTable *symtab;

//ST_Entry thisEntry;

//std::map<char *, SymTabFields> symtable;

BPatch_snippet parse_result;
BPatch_snippet *cParse_result;
%}

%union {
   int  	ival;
   long  lval;
   double dval;
   char 	*sval;
   
   char *context;
   int line_number;

   BPatch_snippet 			*snippet;
   BPatch_boolExpr			*boolExpr;
   BPatch_funcCallExpr			*funcCall;
};

%token <sval> IDENTIFIER MACRO INCLUDE POINTTYPE STRING
%token <dval> NUMBER
%token TRUE FALSE
%token INT VOID LONG BOOL CHAR DOUBLE
%token LOAD VARS PUT AT IF IN WITH
%token REPLACE REMOVE F_CALL F_CALLS TRACE UNTRACE
%token ENABLE DISABLE TOGGLE DETACH REATTACH
%token SNIPPETS MUTATIONS
%token PRINT ABOUT
%token ASSERT KILL BREAK WRITE
%token TERMINATION
%token ERROR

%left COLON
%left ASSIGN
%left SEMI
%left START_BLOCK END_BLOCK
%left START_C_BLOCK END_C_BLOCK

%%

start: IDENTIFIER ASSIGN NUMBER SEMI { printf("%s = %d", $1, $3); /*parse_result = */} ;

%%


#if 0 // For testing the parser independently
extern "C" void set_lex_input(char *str);

int main(int argc, char *argv[])
{
    int ret;

    char str[512];
    printf("here> ");
    gets(str);

    set_lex_input(str);

    ret = yyparse();

    printf("Returned: %d\n", ret);
    return 0;
}
#endif

void yyerror(char *s)
{
    fprintf(stderr, "Error on command line: %s\n",s);
}
