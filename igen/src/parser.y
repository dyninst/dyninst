%{
#include <string.h>
extern int generatePVM;
extern int generateXDR;
#include "parse.h"

#define YYSTYPE union parseStack

int firstTag;
extern int yylex();
extern int emitHeader;
extern int generateTHREAD;
extern char *serverTypeName;
extern void *yyerror(char*);
extern List <typeDefn *> types;
extern typeDefn *foundType;
extern interfaceSpec *currentInterface;
%}

%token tIDENT tCOMMA tARRAY tSTAR tNAME tBASE tINT tUPCALL tASYNC
%token tLPAREN tRPAREN tSEMI tLBLOCK tRBLOCK tCMEMBER tSMEMBER
%token tTYPEDEF tSTRUCT tVERSION tVIRTUAL
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

optUpcall:	   	 { $$.fd.uc = notUpcall; $$.fd.virtual_f = 0; }
	 | tVIRTUAL { $$.fd.uc = notUpcallAsync; $$.fd.virtual_f = 1;}
	 | tASYNC 	 { $$.fd.uc = notUpcallAsync; $$.fd.virtual_f = 0;}
	 | tVIRTUAL tASYNC { $$.fd.uc = notUpcallAsync; $$.fd.virtual_f = 1;}
         | tUPCALL       { $$.fd.uc = syncUpcall; $$.fd.virtual_f = 0;}
         | tVIRTUAL tUPCALL {$$.fd.uc = syncUpcall; $$.fd.virtual_f = 1; }
         | tUPCALL tASYNC { $$.fd.uc = asyncUpcall; $$.fd.virtual_f = 0;}
         | tVIRTUAL tUPCALL tASYNC { $$.fd.uc = asyncUpcall; $$.fd.virtual_f = 1;}

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
	  malloc(strlen($2.td.cp) + 1 + ($3.cl ? $3.cl->count() : 0));
	strcpy(retType, $2.cp);

	for (cl = *$3.cl; *cl; cl++) strcat(retType, "*");

	tf = new remoteFunc(currentInterface, $3.cl, $4.cp, retType, 
	    args, $1.fd.uc, $1.fd.virtual_f, $2.td.structs);
	currentInterface->newMethod(tf);

	if (emitHeader) {
	    tf->genHeader();
	}
      }
         | tCMEMBER tIDENT pointers tIDENT tSEMI {
	   addCMember ($2.cp, $4.cp, $3.cl); }
         |  tSMEMBER typeName pointers tIDENT tSEMI {
	   addSMember ($2.cp, $4.cp, $3.cl);}

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
	| fieldDeclList fieldDecl { $1.fields->add($2.fld); }
	;

// note - $$.td.mallocs will not have any effect for thread code
// td.mallocs -> type will malloc memory
// td.struct -> type is a struct
typeName: tIDENT {
		if (!(foundType = types.find($1.cp)) && !generateTHREAD) {
		    char str[80];
		    sprintf(str, "unknown type %s", $1.cp);
		    yyerror(str);
		}
		$$.td.cp = $1.cp;
		$$.td.structs = 0;
		$$.td.mallocs = 0;
		if (generateTHREAD || (!strcmp("String", $1.cp)))
		  $$.td.mallocs = 1;
		else if (foundType->userDefined == TRUE)
		  {
		    $$.td.mallocs = 1;
		    $$.td.structs = 1;
		  }
	}
	| tARRAY tIDENT {
		extern char *findAndAddArrayType(char*);

		if (!types.find($2.cp) && !generateTHREAD) {
		    char str[80];
		    sprintf(str, "unknown type %s", $2.cp);
		    yyerror(str);
		}
		$$.td.cp = findAndAddArrayType($2.cp);
		$$.td.mallocs = 1;
		$$.td.structs = 1;
	}
	;

fieldDecl: typeName tIDENT tSEMI { 
		$$.fld = new field($1.td.cp, $2.cp);
	 }
	 ;

pointers:		 { $$.cl = new List<char*>; }
	| pointers tSTAR { $1.cl->add("*"); $$.cl = $1.cl; }
	;

argument: typeName pointers tIDENT {
	    $$.arg = new argument($1.td.cp, $3.cp, $2.cl, $1.td.mallocs);
	}
        | typeName pointers {
	    $$.arg = new argument($1.td.cp,currentInterface->genVariable(), $2.cl, $1.td.mallocs);
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
