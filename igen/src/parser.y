%{
#include <string.h>

extern int generatePVM;
extern int generateXDR;
extern int generateTHREAD;
extern int ignoring;
#include "parse.h"

#define YYSTYPE union parseStack

int firstTag;
extern int yylex();
extern int emitHeader;
extern char *serverTypeName;
extern void *yyerror(char*);
extern interfaceSpec *currentInterface;
extern void verify_pointer_use(char *stars, char *retType, char*);
extern int isClass(char *);
extern int isStruct(char *);

%}

%token tIDENT tCOMMA tARRAY tSTAR tNAME tBASE tINT tUPCALL tASYNC
%token tLPAREN tRPAREN tSEMI tLBLOCK tRBLOCK tCMEMBER tSMEMBER
%token tTYPEDEF tSTRUCT tVERSION tVIRTUAL
%token tCLASS tCOLON tIGNORE tLINE
%%

completeDefinition: parsableUnitList { return 0;}
                  | error
                     {
		       char str[80];
		       sprintf(str, "Sorry, unrecoverable error, exiting\n\n");
		       yyerror(str);
		       exit(0);
                     };

parsableUnitList: 
		| parsableUnit { extern int parsing; parsing = 0; }parsableUnitList;

parsableUnit: interfaceSpec 
            | classSpec
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

optUpcall:
                        { $$.fd.uc = syncCall; $$.fd.virtual_f = 0; }
             | tVIRTUAL { $$.fd.uc = asyncCall; $$.fd.virtual_f = 1;}
             | tASYNC 	 { $$.fd.uc = asyncCall; $$.fd.virtual_f = 0;}
             | tVIRTUAL tASYNC { $$.fd.uc = asyncCall; $$.fd.virtual_f = 1;}
             | tUPCALL { $$.fd.uc = syncUpcall; $$.fd.virtual_f = 0; }
             | tVIRTUAL tUPCALL {$$.fd.uc = syncUpcall; $$.fd.virtual_f = 1; }
             | tUPCALL tASYNC { $$.fd.uc = asyncUpcall; $$.fd.virtual_f = 0;}
             | tVIRTUAL tUPCALL tASYNC { $$.fd.uc = asyncUpcall; $$.fd.virtual_f = 1;}

definition: optUpcall typeName pointers tIDENT tLPAREN arglist tRPAREN tSEMI {
	remoteFunc *tf;
	List <argument *> lp, args;

	// this may exit
	verify_pointer_use($3.cp, $2.td.cp, $4.cp);

	/* reverse arg list */
	for (lp = *$6.args; *lp; lp++) {
	    args.add(*lp);
	}

	tf = new remoteFunc(currentInterface, $3.cp, $4.cp, $2.td.cp,
	    args, $1.fd.uc, $1.fd.virtual_f, $2.td.structs);
	currentInterface->newMethod(tf);

	if (emitHeader) {
	    tf->genHeader();
	}
      }
         | tCMEMBER typeName pointers tIDENT tSEMI {
	   addCMember ($2.cp, $4.cp, $3.cp); }
         |  tSMEMBER typeName pointers tIDENT tSEMI {
	   addSMember ($2.cp, $4.cp, $3.cp);}

optIgnore: tIGNORE { $$.cp = $1.cp;}
         | { $$.cp = 0; };

classSpec: tCLASS tIDENT optDerived tLBLOCK fieldDeclList optIgnore tRBLOCK tSEMI
              {
		int slen, i; 
		classDefn *cl;
		List<field *> lp, fields;

		/* reverse field list */
		for (lp = *$5.fields; *lp; lp++) {
		  fields.add(*lp);
		}
		
		$5.fields->removeAll();
		delete ($5.fields);

		// remove ignore tokens
		if ($6.cp) {
		  for (i=0; i<7; i++)
		    ($6.cp)[i] = ' ';
		  slen = strlen($6.cp);
		  for (i=(slen-7); i<slen; ++i)
		    ($6.cp)[i] = ' ';
		}
		cl = new classDefn($2.cp, fields, $3.cp, $6.cp);
		cl->genHeader();
              };

optDerived: {$$.cp = 0;}
          | tCOLON tIDENT {$$.cp = $2.cp;};

typeSpec: tTYPEDEF tSTRUCT tLBLOCK fieldDeclList tRBLOCK tIDENT tSEMI {
	   typeDefn *s;
	   List<field *> lp, fields;

	   /* reverse field list */
	   for (lp = *$4.fields; *lp; lp++) {
		fields.add(*lp);
	   }

	   $4.fields->removeAll();
	   delete ($4.fields);

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
                userDefn *foundType;
		if (generateTHREAD) {
		  $$.td.mallocs = 0;
		  $$.td.structs = 0;
		  $$.td.cp = $1.cp;
		} else { 
		  if (!(foundType = userPool.find($1.cp))) {
		    char str[80];
		    sprintf(str, "unknown type %s, exiting", $1.cp);
		    yyerror(str);
		    exit(0);
		  }
		  $$.td.cp = $1.cp;
		  $$.td.structs = 0;
		  $$.td.mallocs = 0;
		  if (!strcmp("String", $1.cp))
		    $$.td.mallocs = 1;
		  else if (foundType->userDefined == TRUE) {
		    $$.td.mallocs = 1;
		    $$.td.structs = 1;
		  }
		}
	      }
	| tARRAY tIDENT {
	        char str[80];
		extern char *findAndAddArrayType(char*);
		$$.td.cp = findAndAddArrayType($2.cp);
		if (!$$.td.cp) {
		  sprintf(str, "%s cannot be an array, exiting \n", $$.td.cp);
		  yyerror(str);
		  exit(0);
		}
		if (generateTHREAD) {
		  $$.td.mallocs = 0;
		  $$.td.structs = 0;
		} else {
		  $$.td.mallocs = 1;
		  $$.td.structs = 1;
		}
	      };

fieldDecl: typeName tIDENT tSEMI { 
		$$.fld = new field($1.td.cp, $2.cp);
	 }
	 ;

pointers:		 { $$.cp = 0; }
	| pointers tSTAR { 
	                   int len=0, i;
			   if ($1.cp)
			     len = strlen($1.cp);
			   len += 2;
			   $$.cp = new char[len];
			   for (i=0; i<(len-1); ++i)
			     $$.cp[i] = '*';
			   $$.cp[len-1] = (char)0;
	                 }
	;

// when is it ok to use pointers?
// if thread code is generated, it is always ok
// if xdr/pvm code is generated, pointers can be used for:
//       structs
//       classes
// ...but only 1 level of indirection
//
argument: typeName pointers tIDENT {
            // this may exit
            verify_pointer_use($2.cp, $1.td.cp, $3.cp);

	    $$.arg = new argument($1.td.cp, $3.cp, $2.cp, $1.td.mallocs);
	  }
        | typeName pointers {
	    verify_pointer_use($2.cp, $1.td.cp, "no name");

	    $$.arg = new argument($1.td.cp,currentInterface->genVariable(),
				  $2.cp, $1.td.mallocs);
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
