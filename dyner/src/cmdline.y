%{
#if defined(i386_unknown_nt4_0)
#include <malloc.h>
#include <string.h>
#endif

#include "BPatch_thread.h"
#include "BPatch_image.h"
#include "BPatch_type.h"
#include "BPatch_snippet.h"
#include "breakpoint.h"

extern "C" {
void yyerror(char *s);
int yyparse(void);
}

extern BPatch_thread *appThread;
extern BPatch_image *appImage;

int yylex();

BPatch_snippet *parse_result;
parse_ret parse_type;

%}

%union {
    int		   			ival;
    char	   			*sval;
    BPatch_snippet 			*snippet;
    BPatch_boolExpr			*boolExpr;
    BPatch_Vector<BPatch_snippet *>	*snippetList;
    BPatch_funcCallExpr			*funcCall;
};


%token <sval> IDENTIFIER STRING
%token <ival> NUMBER
%token ERROR IF ELSE
%token PLUSPLUS MINUSMINUS

%left DOT DOLLAR
%left OR
%left AND
%left EQ NOT_EQ LESS_EQ GREATER_EQ
%left '+' '-'
%left '*' '/'
%left COMMA
%left ASSIGN
%left SEMI
%left START_BLOCK END_BLOCK

%type <boolExpr> bool_expression 
%type <snippet> arith_expression variable_expr statement inc_decr_expr
%type <snippet> param block
%type <snippetList> param_list statement_list
%type <funcCall> func_call


%%

start:
    '(' bool_expression ')' 
    {
	parse_result = (BPatch_snippet *) $2;
        parse_type = parsed_bool;
    }
    |
    statement_list
    { 
	parse_result = new BPatch_sequence(*$1); 
	parse_type = parsed_statement; 
	delete $1;
    }
    ; 

statement_list: 
    statement
    { 
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
    arith_expression SEMI
    { 
	$$ = $1; 
    }
    | IF '(' bool_expression ')' block 
    {
	$$ = new BPatch_ifExpr(*$3, *$5);
	delete $3;
	delete $5;
    }
    | IF '(' bool_expression ')' block ELSE block 
    {
	$$ = new BPatch_ifExpr(*$3, *$5, *$7);
	delete $3;
	delete $5;
	delete $7;
    }
;

block: statement
     | START_BLOCK statement_list END_BLOCK
     {
	$$ = new BPatch_sequence(*$2);
	delete $2;
     }
;

func_call: IDENTIFIER '(' param_list ')'
    { 
	BPatch_function *func;
	func = appImage->findFunction($1);
	if (!func) {
	    printf("unable to find function %s\n", $1);
	    free($1);
	    return 1;
	}

	free($1);
	$$ = new BPatch_funcCallExpr(*func, *$3); 
    }
    ;

param_list: 
    {
	//No parameters, return an empty vector
	$$ = new BPatch_Vector<BPatch_snippet *>;
    }
    | param 
    { 
	$$ = new BPatch_Vector<BPatch_snippet *>; 
	$$->push_back($1);
    }
    | param_list COMMA param
    { 
	$1->push_back($3); 
	$$ = $1;
    }
    ;

param: arith_expression
    |  STRING
    { 
	$$ = new BPatch_constExpr($1); 
    }
    ;
    

bool_expression: arith_expression '<' arith_expression {
	$$ = new BPatch_boolExpr(BPatch_lt, *$1, *$3);
	delete $1;
	delete $3;
    }
    |		 arith_expression '>' arith_expression {
	$$ = new BPatch_boolExpr(BPatch_gt, *$1, *$3);
	delete $1;
	delete $3;
    }
    |		 arith_expression EQ arith_expression {
	$$ = new BPatch_boolExpr(BPatch_eq, *$1, *$3);
	delete $1;
	delete $3;
    }
    |		 arith_expression LESS_EQ arith_expression {
	$$ = new BPatch_boolExpr(BPatch_le, *$1, *$3);
	delete $1;
	delete $3;
    }
    |		 arith_expression GREATER_EQ arith_expression {
	$$ = new BPatch_boolExpr(BPatch_ge, *$1, *$3);
	delete $1;
	delete $3;
    }
    |		 arith_expression NOT_EQ arith_expression {
	$$ = new BPatch_boolExpr(BPatch_ne, *$1, *$3);
	delete $1;
	delete $3;
    }
    |		 bool_expression AND bool_expression  {
	$$ = new BPatch_boolExpr(BPatch_and, *$1, *$3);
	delete $1;
	delete $3;
    }
    |		 bool_expression OR bool_expression  {
	$$ = new BPatch_boolExpr(BPatch_or, *$1, *$3);
	delete $1;
	delete $3;
    }
    ;

variable_expr:
    IDENTIFIER {
	BPatch_variableExpr *var = findVariable($1);
	if (var == NULL) {
	    fprintf(stderr, "Cannot find variable: %s\n", $1);
	    free($1);
	    return 1;
	    $$ = NULL;
	}
	free($1);
	$$ = var;
    }
    |
    IDENTIFIER DOT IDENTIFIER {
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
	for (int i=0; i < vars->size(); i++) {
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
    | DOLLAR NUMBER {
	if (($2 < 0) || ($2 >= 8)) {
	     printf("parameter %d is not valid\n", $2);
	     return 1;
	}
	$$ = new BPatch_paramExpr($2);
    }
    ;

arith_expression: 
    variable_expr
    |  NUMBER     { $$ = new BPatch_constExpr($1); }
    |		 arith_expression '*' arith_expression  {
	$$ = new BPatch_arithExpr(BPatch_times, *$1, *$3);
	delete $1;
	delete $3;
    }
    |
    func_call { $$ = $1; }
    |
    variable_expr ASSIGN arith_expression {
	    $$ = new BPatch_arithExpr(BPatch_assign, *$1, *$3);
	    if ($1) delete $1;
	    if ($3) delete $3;
    }

    |		 arith_expression '/' arith_expression  {
	$$ = new BPatch_arithExpr(BPatch_divide, *$1, *$3);
	delete $1;
	delete $3;
    }
    |		 arith_expression '+' arith_expression  {
	$$ = new BPatch_arithExpr(BPatch_plus, *$1, *$3);
	delete $1;
	delete $3;
    }
    |		 arith_expression '-' arith_expression  {
	$$ = new BPatch_arithExpr(BPatch_minus, *$1, *$3);
	delete $1;
	delete $3;
    }
    |		 '(' arith_expression ')' {
	$$ = $2;
    }
    |  inc_decr_expr {
	$$ = $1;
    }
    ;

inc_decr_expr:
    variable_expr PLUSPLUS {
	$$ = new BPatch_arithExpr(BPatch_assign, *$1, BPatch_arithExpr(BPatch_plus, 
			*$1, BPatch_constExpr(1)));
    }
    | PLUSPLUS variable_expr {
	$$ = new BPatch_arithExpr(BPatch_assign, *$2, BPatch_arithExpr(BPatch_plus, 
			*$2, BPatch_constExpr(1)));
    }
    | variable_expr MINUSMINUS {
	$$ = new BPatch_arithExpr(BPatch_assign, *$1, BPatch_arithExpr(BPatch_minus, 
			*$1, BPatch_constExpr(1)));
    }
    | MINUSMINUS variable_expr {
	$$ = new BPatch_arithExpr(BPatch_assign, *$2, BPatch_arithExpr(BPatch_minus, 
			*$2, BPatch_constExpr(1)));
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
    fprintf(stderr, "Error on command line: %s\n",s);
}
