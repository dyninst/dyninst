%{
#if defined(i386_unknown_nt4_0)
#include <malloc.h>
#include <string.h>
#endif

/*
 * Yacc will define token IF, conflicting with dyn_regs.h 
 * This undef is safe b/c yacc also creates an enum IF 
 * which serves the same purpose.
*/
#undef IF

#include "BPatch_thread.h"
#include "BPatch_image.h"
#include "BPatch_type.h"
#include "BPatch_snippet.h"
#include "breakpoint.h"

extern "C" {
void yyerror(char *s);
int yyparse(void);
}

class runtimeVar {                                                                                                                                                         
  public:                                                                                                                                                                     
   runtimeVar(BPatch_variableExpr *v, const char *n) {                                                                                                                     
      var = v, name = strdup(n);                                                                                                                                          
   }                                                                                                                                                                       
   BPatch_variableExpr *var;                                                                                                                                               
   char *name;                                                                                                                                                             
   bool readValue(void *buf) { return var->readValue(buf); }                                                                                                               
};

extern BPatch_thread *appThread;
extern BPatch_process *appProc;
extern BPatch_image *appImage;

static BPatch_Vector<runtimeVar *> varList;

const bool verbose = true;

int yylex();

BPatch_snippet *parse_result;
parse_ret parse_type;

%}

%union {
   int  	ival;
   long  lval;
   double dval;
   char 	*sval;
   
   BPatch_snippet 			*snippet;
   BPatch_boolExpr			*boolExpr;
   BPatch_Vector<BPatch_snippet *>	*snippetList;
   BPatch_funcCallExpr			*funcCall;
   BPatch_variableExpr     *varExpr;
};


%token <sval> TYPE IDENTIFIER STRING
%token <ival> NUMBER
%token TRUE FALSE
%token QUESTION_MARK COLON
%token _ERROR IF ELSE
%token PLUSPLUS MINUSMINUS

%left DOT DOLLAR ASTERISK AMPERSAND
%left NOT
%left OR
%left AND
%left EQ NOT_EQ LESS_EQ GREATER_EQ
%left ASSIGN ADD_ASSIGN SUB_ASSIGN DIV_ASSIGN MUL_ASSIGN MOD_ASSIGN
%left '?'
%left ':'
%left '+' '-'
%left '*' '/' '%'
%left COMMA
%left SEMI
%left START_BLOCK END_BLOCK

%type <boolExpr> bool_expression bool_constant
%type <snippet> arith_expression  statement inc_decr_expr 
%type <snippet> param block
%type <snippetList> param_list statement_list
%type <funcCall> func_call
%type <varExpr> variable_expr


%%

start:
    '(' bool_expression ')' 
    {
       if(verbose) printf(" <bool>\n");
       parse_result = $2;
       parse_type = parsed_bool;
    }
    | statement_list
    { 
       parse_result = new BPatch_sequence(*$1); 
       parse_type = parsed_statement; 
       delete $1;
    }
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
    ;

statement:
    // 1 + 2;
    arith_expression SEMI
    { 
       $$ = $1; 
    }
  // if (x == y) {}
    | IF '(' bool_expression ')' block 
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
    ;

block: statement
     // {stuff}
     | START_BLOCK statement_list END_BLOCK
     {
        $$ = new BPatch_sequence(*$2);
        delete $2;
     }
     ;

func_call: IDENTIFIER '(' param_list ')'
           //funct(stuff,stuff)
    { 
       if(verbose) printf(" %s () ", $1);
       BPatch_Vector<BPatch_function *>bpfv;
       if (NULL == appImage->findFunction($1, bpfv) || !bpfv.size()) {
          printf("unable to find function %s\n", $1);
          free($1);
          return 1;
       }
       if (bpfv.size() > 1) {
// make this non verbose -- have it return errors through proper channels or as part of generated code.
          printf("found %d references to  %s\n", (int) bpfv.size(), $1);
       }

       BPatch_function *func = bpfv[0];
	
       if (!func) {
          printf("unable to find function %s\n", $1);
          free($1);
          return 1;
       }
      
       free($1);
       $$ = new BPatch_funcCallExpr(*func, *$3); 
       delete $3;
    }
    ;

param_list: 
    //()
    {
       //No parameters, return an empty vector
       $$ = new BPatch_Vector<BPatch_snippet *>;
    }
    //(stuff)
    | param 
    { 
       $$ = new BPatch_Vector<BPatch_snippet *>; 
       $$->push_back($1);
    }
    //(stuff,stuff)
    | param_list COMMA param
    {
       if(verbose) printf(" , ");
       $1->push_back($3); 
       $$ = $1;
    }
    ;

param: arith_expression  //funct(2+5)
    //funct("hi")
    |  STRING { 
       $$ = new BPatch_constExpr($1); 
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
       delete $1;
       delete $3;
    }
    |	arith_expression '>' arith_expression
    {
       if(verbose) printf(" > ");
       $$ = new BPatch_boolExpr(BPatch_gt, *$1, *$3);
       delete $1;
       delete $3;
    }
    | arith_expression EQ arith_expression
    {
       if(verbose) printf(" == ");
       $$ = new BPatch_boolExpr(BPatch_eq, *$1, *$3);
       delete $1;
       delete $3;
    }
    | arith_expression LESS_EQ arith_expression
    {
       if(verbose) printf(" <= ");
       $$ = new BPatch_boolExpr(BPatch_le, *$1, *$3);
       delete $1;
       delete $3;
    }
    | arith_expression GREATER_EQ arith_expression 
    {
       if(verbose) printf(" >= ");
       $$ = new BPatch_boolExpr(BPatch_ge, *$1, *$3);
       delete $1;
       delete $3;
    }
    | arith_expression NOT_EQ arith_expression 
    {
       if(verbose) printf(" != ");
       $$ = new BPatch_boolExpr(BPatch_ne, *$1, *$3);
       delete $1;
       delete $3;
    }
    |	bool_expression AND bool_expression  
    {
       if(verbose) printf(" AND ");
       $$ = new BPatch_boolExpr(BPatch_and, *$1, *$3);
       delete $1;
       delete $3;
    }
    | bool_expression OR bool_expression  
    {       if(verbose) printf(" OR ");
       $$ = new BPatch_boolExpr(BPatch_or, *$1, *$3);
       delete $1;
       delete $3;
    }
    // add not
    ;

variable_expr: IDENTIFIER
    {
       BPatch_variableExpr *var = findVariable($1);
       if (var == NULL) {
          fprintf(stderr, "Cannot find variable: %s\n", $1);
          free($1);
          return 1;
          $$ = NULL;
       }
       if(verbose) printf(" var(%s) ", $1);
       free($1);
       $$ = var;
    }
    | IDENTIFIER DOT IDENTIFIER
    {
       bool foundField = false;
       
       BPatch_variableExpr *var = findVariable($1);
       if (var == NULL) {
          fprintf(stderr, "Cannot find variable: %s\n", $1);
          free($1);
          return 1;
       }

       BPatch_Vector<BPatch_variableExpr *> *vars = var->getComponents();
       if (!vars) {
          fprintf(stderr, "is not an aggregate type: %s\n", $1);
          free($1);
          return 1;
       }
       for (unsigned int i=0; i < vars->size(); i++) {
          if (!strcmp($3, (*vars)[i]->getName())) {
             $$ = (*vars)[i];
             foundField = true;
          }
       }

       if (!foundField) {
          fprintf(stderr, "%s is not a field of %s\n", $3, $1);
          free($1);
          return 1;
       }

       free($1);
    }
    ;
/*
    | DOLLAR NUMBER
    {
       if (($2 < 0) || ($2 >= 8)) {
          printf("parameter %d is not valid\n", $2);
          return 1;
       }
       $$ = new BPatch_paramExpr($2);
    }
    ;
*/
arith_expression: variable_expr { $$ = (BPatch_snippet *)$1;}
    | bool_expression '?' arith_expression ':' arith_expression
    {
       //can't allow arith_expressions that don't return a val, i.e. assigns
       //type checking ?
       // throw away expression:
       BPatch_arithExpr retExpr = BPatch_arithExpr(BPatch_negate, BPatch_constExpr(1));
       BPatch_arithExpr assignFirst = BPatch_arithExpr(BPatch_assign, retExpr, *$3);
       BPatch_arithExpr assignSecond = BPatch_arithExpr(BPatch_assign, retExpr, *$5);
      
       $$ = new BPatch_arithExpr(BPatch_seq, BPatch_ifExpr(*$1, assignFirst, assignSecond), retExpr);
    }
    | NUMBER     { if(verbose) printf(" %d ", $1); $$ = new BPatch_constExpr($1);}
    |	arith_expression '*' arith_expression 
    {
       if(verbose) printf(" * ");
       $$ = new BPatch_arithExpr(BPatch_times, *$1, *$3);
       delete $1;
       delete $3;
    }
    | func_call {$$ = $1;}
    | variable_expr ASSIGN arith_expression
    {
       if(verbose) printf(" = ");
	    $$ = new BPatch_arithExpr(BPatch_assign, *$1, *$3);
	    delete $1;
	    delete $3;
    }

    | variable_expr ADD_ASSIGN arith_expression
    {
       if(verbose) printf(" += ");
       $$ = new BPatch_arithExpr(BPatch_assign, *$1, BPatch_arithExpr(BPatch_plus, *$1, *$3));
       delete $1;
       delete $3;
    }
    | variable_expr SUB_ASSIGN arith_expression
    {
       if(verbose) printf(" -= ");
       $$ = new BPatch_arithExpr(BPatch_assign, *$1, BPatch_arithExpr(BPatch_minus, *$1, *$3));
       delete $1;
       delete $3;
    }
    | variable_expr MUL_ASSIGN arith_expression
    {
       if(verbose) printf(" *= ");
       $$ = new BPatch_arithExpr(BPatch_assign, *$1, BPatch_arithExpr(BPatch_times, *$1, *$3));
       delete $1;
       delete $3;
    }
    | variable_expr DIV_ASSIGN arith_expression
    {
       if(verbose) printf(" /= ");
       $$ = new BPatch_arithExpr(BPatch_assign, *$1, BPatch_arithExpr(BPatch_divide, *$1, *$3));
       delete $1;
       delete $3;
    }
    | variable_expr MOD_ASSIGN arith_expression
    {
       if(verbose) printf(" %%= ");
       $$ = new BPatch_arithExpr(BPatch_assign, *$1, BPatch_arithExpr(BPatch_minus, *$1, BPatch_arithExpr(BPatch_times, BPatch_arithExpr(BPatch_divide, *$1, *$3), *$3)));
       delete $1;
       delete $3;
    }

    |	arith_expression '/' arith_expression 
    {
       if(verbose) printf(" / ");
       $$ = new BPatch_arithExpr(BPatch_divide, *$1, *$3);
       delete $1;
       delete $3;
    }
    | arith_expression '%' arith_expression
    {
       if(verbose) printf(" %% ");
       $$ = new BPatch_arithExpr(BPatch_minus, *$1, BPatch_arithExpr(BPatch_times, BPatch_arithExpr(BPatch_divide, *$1, *$3), *$3));
       delete $1;
       delete $3;
    }
    |	arith_expression '+' arith_expression 
    {
       if(verbose) printf(" + ");
       $$ = new BPatch_arithExpr(BPatch_plus, *$1, *$3);
       delete $1;
       delete $3;
    }
    |	arith_expression '-' arith_expression 
    {
       if(verbose) printf(" - ");
       $$ = new BPatch_arithExpr(BPatch_minus, *$1, *$3);
       delete $1;
       delete $3;
    }
    |	'(' arith_expression ')'  {$$ = $2;}
    | inc_decr_expr {$$ = $1;}
     
    ;

inc_decr_expr:
    variable_expr PLUSPLUS 
    {
       if(verbose) printf(" <++ ");
       BPatch_arithExpr addOne = BPatch_arithExpr(BPatch_assign, *$1, BPatch_arithExpr(BPatch_plus, *$1, BPatch_constExpr(1)));
       $$ = new BPatch_arithExpr(BPatch_seq, (BPatch_snippet &)addOne, BPatch_arithExpr(BPatch_minus, *$1, BPatch_constExpr(1)));
    }
    | PLUSPLUS variable_expr 
    {
       if(verbose) printf(" ++> ");
       BPatch_arithExpr addOne = BPatch_arithExpr(BPatch_assign, *$2, BPatch_arithExpr(BPatch_plus, *$2, BPatch_constExpr(1)));
       $$ = new BPatch_arithExpr(BPatch_seq, (BPatch_snippet &)addOne, *$2);
    }
    | variable_expr MINUSMINUS
    {
       if(verbose) printf(" -- ");
       BPatch_arithExpr subOne = BPatch_arithExpr(BPatch_assign, *$1, BPatch_arithExpr(BPatch_minus, *$1, BPatch_constExpr(1)));
       $$ = new BPatch_arithExpr(BPatch_seq, (BPatch_snippet &)subOne, BPatch_arithExpr(BPatch_plus, *$1, BPatch_constExpr(1)));
    }
    | MINUSMINUS variable_expr
    {
       if(verbose) printf(" -- ");
       BPatch_arithExpr subOne = BPatch_arithExpr(BPatch_assign, *$2, BPatch_arithExpr(BPatch_minus, *$2, BPatch_constExpr(1)));
       $$ = new BPatch_arithExpr(BPatch_seq, (BPatch_snippet &)subOne, *$2);
    }
    ;

%%


#if 0 // For testing the parser independently
extern "C" void set_lex_input(char *str);

main(int argc, char *argv[])
{
    int ret;

    char str[512];
    printf("here> ");
    gets(str);

    set_lex_input(str);

    ret = yyparse();

    printf("Returned: %d\n", ret);
}
#endif



void yyerror(char *s)
{
   fflush(stderr);
    fprintf(stderr, "Error on command line: %s\n",s);
}
