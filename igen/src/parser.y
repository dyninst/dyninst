%{
#include <string.h>
#include "parse.h"

#define YYSTYPE union parseStack

int firstTag;
extern int yylex();
extern int emitHeader;
extern int generateTHREAD;
extern char *serverTypeName;
extern void *yyerror(char*);
extern List <typeDefn *> types;
extern interfaceSpec *currentInterface;
%}

%token tIDENT tCOMMA tARRAY tSTAR tNAME tBASE tINT tUPCALL tASYNC
%token tLPAREN tRPAREN tSEMI tLBLOCK tRBLOCK
%token tTYPEDEF tSTRUCT tVERSION 
%%

completeDefinition: parsableUnitList ;

parsableUnitList: 
		| parsableUnit { extern int parsing; parsing = 0; }parsableUnitList;

parsableUnit: interfaceSpec 
	    | typeSpec;

interfacePreamble: interfaceName tLBLOCK interfaceBase interfaceVersion {
    interfaceSpec *is;

    is = new interfaceSpec($1.cp, $4.i, $3.i);
    currentInterface = $$.spec = is;
} 

interfaceSpec: interfacePreamble definitionList tRBLOCK tSEMI {
    $1.spec->genClass();
}

interfaceName: tIDENT { $$.cp = $1.cp; };

interfaceBase: tBASE tINT tSEMI { $$.i = $2.i; };

interfaceVersion: tVERSION tINT tSEMI { $$.i = $2.i; };

definitionList:
	      | definitionList definition
	      ;

optUpcall:	   	 { $$.uc = notUpcall; }
	 | tUPCALL 	 { $$.uc = syncUpcall; }
	 | tUPCALL tASYNC { $$.uc = asyncUpcall; }

definition: optUpcall typeName pointers tIDENT tLPAREN arglist tRPAREN tSEMI {
	char *retType;
	remoteFunc *tf;
	List <char *> cl;
	List <argument *> lp, args;

	/* reverse arg list */
	for (lp = *$6.args; *lp; lp++) {
	    args.add(*lp);
	}
	retType = (char *) 
	    malloc(strlen($2.cp) + 1 + ($3.cl ? $3.cl->count() : 0));
	strcpy(retType, $2.cp);
	for (cl = *$3.cl; *cl; cl++) strcat(retType, "*");

	tf = new remoteFunc(currentInterface, $3.cl, $4.cp, retType, 
	    args, $1.uc);
	currentInterface->newMethod(tf);

	if (emitHeader) {
	    tf->genHeader();
	}
    }


typeSpec: tTYPEDEF tSTRUCT tLBLOCK fieldDeclList tRBLOCK tIDENT tSEMI {
	   typeDefn *s;
	   List<field *> lp, fields;

	   /* reverse field list */
	   for (lp = *$4.fields; *lp; lp++) {
		fields.add(*lp);
	   }
	   s = new typeDefn($6.cp, fields);
	   s->genHeader();
       }
   ;

fieldDeclList:	{ $$.fields = new List<field*>; }
	| fieldDeclList fieldDecl { $1.fields->add($2.field); }
	;

typeName: tIDENT {
		if (!types.find($1.cp) && !generateTHREAD) {
		    char str[80];
		    sprintf(str, "unknown type %s", $1.cp);
		    yyerror(str);
		}
		$$.cp = $1.cp;
	}
	| tARRAY tIDENT {
		extern char *findAndAddArrayType(char*);

		if (!types.find($2.cp) && !generateTHREAD) {
		    char str[80];
		    sprintf(str, "unknown type %s", $2.cp);
		    yyerror(str);
		}
		$$.cp = findAndAddArrayType($2.cp);
	}
	;

fieldDecl: typeName tIDENT tSEMI { 
		$$.field = new field($1.cp, $2.cp);
	 }
	 ;

pointers:		 { $$.cl = new List<char*>; }
	| pointers tSTAR { $1.cl->add("*"); $$.cl = $1.cl; }
	;

argument: typeName pointers tIDENT {
	    $$.arg = new argument($1.cp, $3.cp, $2.cl);
	}
        | typeName pointers {
	    $$.arg = new argument($1.cp,currentInterface->genVariable(), $2.cl);
	}
	;

nonEmptyArg: argument	{    
		$$.args = new(List<argument*>);
		$$.args->add($1.arg);
	    }
           | nonEmptyArg tCOMMA argument {
	       $1.args->add($3.arg);
	   }
	   ;

arglist:		{ $$.args = new(List<argument*>); }
       | nonEmptyArg	{ $$.args = $1.args; }
       ;
