%{
#include <string.h>

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
extern int isClass(stringHandle);
extern int isStruct(stringHandle);

%}

%token tIDENT tCOMMA tARRAY tSTAR tNAME tBASE tINT tUPCALL tASYNC
%token tLPAREN tRPAREN tSEMI tLBLOCK tRBLOCK tCMEMBER tSMEMBER
%token tTYPEDEF tSTRUCT tVERSION tVIRTUAL
%token tCLASS tCOLON tIGNORE tLINE tCONST tDONTFREE
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
            | typeSpec
            | arraySpec;

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

optDontFree: { $$.i=0;}
           |  tDONTFREE { $$.i = 1;};

definition: optUpcall optDontFree typeName pointers tIDENT tLPAREN arglist tRPAREN tSEMI {
	remoteFunc *tf;
	List <argument *> lp, args;

	/* reverse arg list */
	for (lp = *$7.args; *lp; ++lp) {
	    args.add(*lp);
	}

	tf = new remoteFunc(currentInterface, $4.charp, $5.cp, 
	    args, $1.fd.uc, $1.fd.virtual_f, $3.td.f_type, $2.i);
	currentInterface->newMethod(tf);

	if (emitHeader) {
	    tf->genHeader();
	}
      }
         | tCMEMBER typeName pointers tIDENT tSEMI {
	   addCMember ($2.td.cp, $4.cp, $3.charp); }
         |  tSMEMBER typeName pointers tIDENT tSEMI {
	   addSMember ($2.td.cp, $4.cp, $3.charp);}

optIgnore: tIGNORE { $$.charp = $1.charp;}
         | { $$.charp = 0; };

classSpec: tCLASS tIDENT optDerived tLBLOCK fieldDeclList optIgnore tRBLOCK tSEMI
              {
		int slen, i; 
		classDefn *cl=0; userDefn *f=0;
		List<field *> lp, fields;
		
		/* reverse field list */
		for (lp = *$5.fields; *lp; ++lp) {
		  fields.add(*lp);
		}
		
		$5.fields->removeAll();
		delete ($5.fields);

		// remove ignore tokens
		if ($6.charp) {
		  for (i=0; i<7; i++)
		    ($6.charp)[i] = ' ';
		  slen = strlen($6.charp);
		  for (i=(slen-7); i<slen; ++i)
		    ($6.charp)[i] = ' ';
		}
		if ($3.cp) {
		  f = userPool.find($3.cp);
		  if (f->whichType() != CLASS_DEFN) {
		    char str[80];
		    sprintf(str, "unknown parent class %s, exiting", (char*)$3.cp);
		    yyerror(str);
		  }
		  cl = (classDefn*) f;
		}
		cl = new classDefn($2.cp, fields, cl, $6.charp);
		cl->genHeader();
              };

optDerived: {$$.cp = 0;}
          | tCOLON tIDENT {$$.cp = $2.cp;};

arraySpec: tARRAY typeName pointers tIDENT tSEMI {
  char str[80];
  userDefn *ptrD;

  if ($3.charp) {
    ptrD = new typeDefn (($2.td.f_type)->getUsh(), $3.charp);
    ptrD->genHeader();
  }
  else
    ptrD = $2.td.f_type;

  if (!ptrD) {
    sprintf(str, "%s cannot be an array, bad element, exiting \n", $4.cp);
    yyerror(str);
    exit(0);
  }
  typeDefn *newType = new typeDefn($4.cp, ptrD, 0);
  if (!newType) {
    sprintf(str, "%s cannot be an array, exiting \n", $4.cp);
    yyerror(str);
    exit(0);
  }
  newType->genHeader();
};

typeSpec: tTYPEDEF tSTRUCT tLBLOCK fieldDeclList tRBLOCK tIDENT tSEMI {
	   typeDefn *s;
	   List<field *> lp, fields;

	   /* reverse field list */
	   for (lp = *$4.fields; *lp; ++lp) {
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

typeName: tIDENT {
                userDefn *foundType=0;

		if (!(foundType = userPool.find($1.cp))) {
		  char str[80];

		  if (generateTHREAD) {
		    foundType = new typeDefn($1.cp);
		  } else {
		    sprintf(str, "unknown type %s, exiting", (char*)$1.cp);
		    yyerror(str);
		    exit(0);
		  }
		}
		$$.td.f_type = foundType;
		$$.td.cp = $1.cp;
	      };

optConst:  { $$.i = 0; }
        |  tCONST { $$.i = 1;};

fieldDecl: optConst typeName pointers tIDENT tSEMI { 
		$$.fld = new field($4.cp, $3.charp, $1.i, $2.td.f_type);
	 }
	 ;

pointers:		 { $$.charp = 0; }
	| pointers tSTAR { 
	                   int len=0, i;
			   if ($1.charp)
			     len = strlen($1.charp);
			   len += 2;
			   $$.charp = new char[len];
			   for (i=0; i<(len-1); ++i)
			     $$.charp[i] = '*';
			   $$.charp[len-1] = (char)0;
	                 }
	;

// when is it ok to use pointers?
// if thread code is generated, it is always ok
// if xdr/pvm code is generated, pointers can be used for:
//       structs
//       classes
// ...but only 1 level of indirection
//
argument: optConst typeName pointers tIDENT {
             // this may exit
              $$.arg = new argument($4.cp, $3.charp, $1.i, $2.td.f_type);
	    }
        | optConst typeName pointers {
	    $$.arg = new argument(currentInterface->genVariable(),
				  $3.charp, $1.i, $2.td.f_type);
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
