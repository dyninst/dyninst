
%{ 

/*
 * $Log: metParser.y,v $
 * Revision 1.4  1995/02/16 08:24:21  markc
 * Changed Boolean to bool.
 * Changed calls to igen functions to use strings/vectors rather than
 * char*'s/arrays
 *
 * Revision 1.3  1995/02/07  21:59:54  newhall
 * added a force option to the visualization definition, this specifies
 * if the visi should be started before metric/focus menuing
 * removed compiler warnings
 *
 * Revision 1.2  1994/08/22  15:53:26  markc
 * Config language version 2.
 *
 * Revision 1.1  1994/07/07  03:25:27  markc
 * Configuration language parser.
 *
 */

#include "paradyn/src/met/metParse.h"
#include <string.h>

#define YYSTYPE struct parseStack

extern int yylex();
extern void *yyerror(char *);
extern int yyparse();
extern void handle_error();
%}

%token tDAEMON tPROCESS tTUNABLE_CONSTANT tIDENT tINT
%token tCOMMAND tARGS tHOST tLITERAL tFLOAT tCOMMA
%token tSEMI tLBRAC tRBRAC tFLAVOR tPVM tCM5 tUNIX tNAME
%token tLIST tVISI tUSER tDIR tFALSE tTRUE tFORCE

%%

stuff: definitions { yyclearin; return(0); };

definitions:
           | definition definitions;


definition: daemonDef
	  | processDef
	  | tunableConstant
          | stringList
	  | visiDef
	  | error;

stringList: tLIST tIDENT tLBRAC stringItems tRBRAC { 
		 metParseError = ERR_NO_ERROR;
		 if (!stringList::addStringList($2.cp, *$4.sl)) {
		   handle_error();
		 }
	       }
          | tLIST error;

stringItems:     { $$.sl = new List<char*>;}
           | stringItems tLITERAL
                 { $$.sl = $1.sl; ($$.sl)->add($2.cp); };

visiDef: tVISI tIDENT visiStruct {
             field f;
	     f.val = $2.cp; f.spec = SET_NAME;
             ($3.vm)->set_field(f);
	     metParseError = ERR_NO_ERROR;
	     if (!visiMet::addVisi($3.vm)) {
	       handle_error();
	     }
           }
       | tVISI error;

visiStruct: tLBRAC vItems tRBRAC {$$.vm = $2.vm;};

vItems:      { $$.vm = new visiMet();}
       | vItems vItem
             { $$.vm = $1.vm; ($$.vm)->set_field($2.fld);};

vItem: tCOMMAND tLITERAL tSEMI
           { $$.fld.val = $2.cp; $$.fld.spec = SET_COMMAND;}
     | tHOST tIDENT tSEMI
           { $$.fld.val = $2.cp; $$.fld.spec = SET_HOST;}
     | tUSER tIDENT tSEMI
           { $$.fld.val = $2.cp; $$.fld.spec = SET_USER;}
     | tDIR tIDENT tSEMI
           { $$.fld.val = $2.cp; $$.fld.spec = SET_DIR;};
     | tFORCE tINT tSEMI
           { $$.fld.flav = $2.i; $$.fld.spec = SET_FORCE;};

processDef: tPROCESS tIDENT processItem {
               field f;
	       f.val = $2.cp; f.spec = SET_NAME;
	       ($3.pm)->set_field(f);
	       metParseError = ERR_NO_ERROR;
	       if (!processMet::addProcess($3.pm)) {
	         handle_error();
	       }
             }
          | tPROCESS error;

processItem: tLBRAC aItems tRBRAC {$$.pm = $2.pm; };

aItems:      { $$.pm = new processMet();}
       |  aItems aItem
	     { $$.pm = $1.pm; ($$.pm)->set_field($2.fld);};

aItem: tCOMMAND tLITERAL tSEMI
           { $$.fld.val = $2.cp; $$.fld.spec = SET_COMMAND;}
     | tDAEMON tIDENT tSEMI
           { $$.fld.val = $2.cp; $$.fld.spec = SET_DAEMON;}
     | tHOST tIDENT tSEMI
           { $$.fld.val = $2.cp; $$.fld.spec = SET_HOST;}
     | tUSER tIDENT tSEMI
           { $$.fld.val = $2.cp; $$.fld.spec = SET_USER;}
     | tDIR tIDENT tSEMI
           { $$.fld.val = $2.cp; $$.fld.spec = SET_DIR;};

tunableConstant: tTUNABLE_CONSTANT tunableItem
               | tTUNABLE_CONSTANT tLBRAC tunableList tRBRAC
	       | tTUNABLE_CONSTANT tLBRAC error tRBRAC
	       | tTUNABLE_CONSTANT tLBRAC error
	       | tTUNABLE_CONSTANT error;

tunableList: 
           | tunableList tunableItem;

tunableItem: tLITERAL tINT tSEMI  {
                 metParseError = ERR_NO_ERROR;
                 if (!tunableMet::addTunable($1.cp, (float) $2.i))
		   handle_error();
	       }
           | tLITERAL tFLOAT tSEMI {
	         metParseError = ERR_NO_ERROR;
	         if (!tunableMet::addTunable($1.cp, $2.f))
                   handle_error();
	       }
           | tLITERAL tTRUE tSEMI {
                 metParseError = ERR_NO_ERROR;
                 if (!tunableMet::addTunable($1.cp, true))
		   handle_error();
	       }
           | tLITERAL tFALSE tSEMI {
                 metParseError = ERR_NO_ERROR;
                 if (!tunableMet::addTunable($1.cp, false))
		   handle_error();
	       };

daemonDef: tDAEMON tIDENT daemonStruct {
             field f;
	     f.val = $2.cp; f.spec = SET_NAME;
	     ($3.dm)->set_field(f);
	     metParseError = ERR_NO_ERROR;
	     if (!daemonMet::addDaemon($3.dm)) {
	       handle_error();
	     }
	   }
	 | tDAEMON error;

daemonStruct: tLBRAC dStructItems tRBRAC {$$.dm = $2.dm;};

dStructItems:    { $$.dm = new daemonMet();}
            |  dStructItems daemonItem
                 { $$.dm = $1.dm; ($$.dm)->set_field($2.fld);};

daemonItem:  tCOMMAND tLITERAL tSEMI
               { $$.fld.val = $2.cp; $$.fld.spec = SET_COMMAND;}
           | tUSER tIDENT tSEMI
               { $$.fld.val = $2.cp; $$.fld.spec = SET_USER;}
           | tFLAVOR flavor tSEMI
               { $$.fld.flav = $2.flav; $$.fld.spec = SET_FLAVOR;}
           | tHOST tIDENT tSEMI
               { $$.fld.val = $2.cp; $$.fld.spec = SET_HOST;}
           | tDIR tIDENT tSEMI
              { $$.fld.val = $2.cp; $$.fld.spec = SET_DIR;};

flavor: tPVM { $$.flav = metPVM;}
      | tUNIX { $$.flav = metUNIX;}
      | tCM5 { $$.flav = metCM5;};



