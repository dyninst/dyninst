

%{ 

/*
 * $Log: metParser.y,v $
 * Revision 1.1  1994/07/07 03:25:27  markc
 * Configuration language parser.
 *
 */

#include "paradyn/src/met/metParse.h"
#include <string.h>

#define YYSTYPE struct parseStack

extern int yylex();
extern void *yyerror(char *);
extern int yyparse();
extern int metProcess (processStruct);
extern int metVisi (visiStruct);
extern int metTunable (char*, float);
extern int metDaemon (daemonStruct);
%}

%token tDAEMON tPROCESS tTUNABLE_CONSTANT tIDENT tINT
%token tCOMMAND tARGS tHOST tLITERAL tFLOAT tCOMMA
%token tSEMI tLBRAC tRBRAC tFLAVOR tPVM tCM5 tNAME
%token tLIST tVISI

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

stringList: tLIST tLITERAL tLBRAC stringItems tRBRAC
                {$$.sl += $4.sl;};

noNameStringList: tLBRAC stringItems tRBRAC
                   {$$.sl += $2.sl;}
		| tLBRAC tRBRAC;

stringItems: tLITERAL {$$.sl.add($1.cp);}
           | tLITERAL stringItems {$$.sl.add($1.cp); $$.sl += $2.sl;};

visiDef: tVISI visiRest;

visiRest: visiStruct
	| tLBRAC visiStructs tRBRAC
	| tLBRAC error tRBRAC
	| tLBRAC error
	| error;

visiStructs: visiStruct
	   | visiStruct visiStructs;

visiStruct: tLBRAC vItems tRBRAC
	      {
		  metVisi($2.vs);
              };

vItems: vItem 
           {
	     if ($1.vs.name)
	       $$.vs.name = $1.vs.name;
	     else if ($1.vs.command)
	       $$.vs.command = $1.vs.command;
	     else if ($1.vs.args_used)
	       $$.vs.args = $1.vs.args;
	     else if ($1.vs.host)
	       $$.vs.host = $1.vs.host;
	   }
      | vItem vItems
           {
	     $$.vs.name = $2.vs.name;
	     $$.vs.command = $2.vs.command;
	     $$.vs.args = $2.vs.args;
	     $$.vs.host = $2.vs.host;

	     if ($1.vs.name)
	       $$.vs.name = $1.vs.name;
	     else if ($1.vs.command)
	       $$.vs.command = $1.vs.command;
	     else if ($1.vs.args_used)
	       $$.vs.args = $1.vs.args;
	     else if ($1.vs.host)
	       $$.vs.host = $1.vs.host;
	   };

vItem: tNAME tLITERAL tSEMI
           { $$.vs.name = $2.cp;
	     $$.vs.command = 0; $$.vs.host = 0;
	     $$.vs.args_used = 0; }
     | tCOMMAND tLITERAL tSEMI
           { $$.vs.command = $2.cp;
	     $$.vs.name = 0; $$.vs.host = 0;
	     $$.vs.args_used = 0; }
     | tARGS noNameStringList tSEMI
           { $$.vs.args = $2.sl; $$.vs.args_used = 1;
	     $$.vs.host = 0; $$.vs.command = 0;
	     $$.vs.name = 0; }
     | tHOST tLITERAL tSEMI
           { $$.vs.host = $2.cp;
	     $$.vs.command = 0; $$.vs.args_used = 0;
	     $$.vs.name = 0; };

processDef: tPROCESS processRest;

processRest: processItem
           | tLBRAC processItems tRBRAC
	   | tLBRAC error tRBRAC
	   | tLBRAC error 
	   | error;

processItems: processItem 
            | processItem processItems;

processItem: tLBRAC aItems tRBRAC
                  {
                      metProcess($2.ps);
                  };

aItems: aItem 
           {
	     if ($1.ps.name)
	       $$.ps.name = $1.ps.name;
	     else if ($1.ps.command)
	       $$.ps.command = $1.ps.command;
	     else if ($1.ps.args_used)
	       $$.ps.args = $1.ps.args;
	     else if ($1.ps.host)
	       $$.ps.host = $1.ps.host;
	     else if ($1.ps.daemon)
	       $$.ps.daemon = $1.ps.daemon;
	     else $$.ps.flavor = $1.ps.flavor;
	   }
      | aItem aItems
           {
	     $$.ps.name = $2.ps.name;
	     $$.ps.command = $2.ps.command;
	     $$.ps.args = $2.ps.args;
	     $$.ps.host = $2.ps.host;
	     $$.ps.daemon = $2.ps.daemon;
	     $$.ps.flavor = $2.ps.flavor;

	     if ($1.ps.name)
	       $$.ps.name = $1.ps.name;
	     else if ($1.ps.command)
	       $$.ps.command = $1.ps.command;
	     else if ($1.ps.args_used)
	       $$.ps.args = $1.ps.args;
	     else if ($1.ps.host)
	       $$.ps.host = $1.ps.host;
	     else if ($1.ps.daemon)
	       $$.ps.daemon = $1.ps.daemon;
	     else $$.ps.flavor = $1.ps.flavor;
	   };

aItem: tNAME tLITERAL tSEMI
           { $$.ps.name = $2.cp;
	     $$.ps.command = 0; $$.ps.host = 0;
	     $$.ps.args_used = 0; $$.ps.daemon = 0; }
     | tCOMMAND tLITERAL tSEMI
           { $$.ps.command = $2.cp;
	     $$.ps.name = 0; $$.ps.host = 0;
	     $$.ps.args_used = 0; $$.ps.daemon = 0;}
     | tARGS noNameStringList tSEMI
           { $$.ps.args = $2.sl; $$.ps.args_used = 1;
	     $$.ps.host = 0; $$.ps.command = 0;
	     $$.ps.name = 0; $$.ps.daemon = 0; }
     | tHOST tLITERAL tSEMI
           { $$.ps.host = $2.cp;
	     $$.ps.command = 0; $$.ps.args_used = 0;
	     $$.ps.name = 0; $$.ps.daemon = 0; }
     | tDAEMON tLITERAL tSEMI
           { $$.ps.daemon = $2.cp;
	     $$.ps.command = 0; $$.ps.args_used = 0;
	     $$.ps.name = 0; $$.ps.host = 0; }
     | tFLAVOR flavor tSEMI
           { $$.ps.flavor = $2.flav;
	     $$.ps.name = 0; $$.ps.args_used = 0; $$.ps.daemon = 0;
	     $$.ps.host = 0; $$.ps.command = 0};

tunableConstant: tTUNABLE_CONSTANT tunableItem
               | tTUNABLE_CONSTANT tLBRAC tunableList tRBRAC
	       | tTUNABLE_CONSTANT tLBRAC error tRBRAC
	       | tTUNABLE_CONSTANT tLBRAC error
	       | tTUNABLE_CONSTANT error;

tunableList: tunableItem
           | tunableItem tunableList;

tunableItem: tLITERAL tINT tSEMI { metTunable($1.cp, (float) $2.i);}
           | tLITERAL tFLOAT tSEMI {metTunable($1.cp, (float) $2.i);};

daemonDef: tDAEMON daemonStruct
         | tDAEMON tLBRAC daemonList tRBRAC
	 | tDAEMON tLBRAC error tRBRAC
	 | tDAEMON tLBRAC error
	 | tDAEMON error;

daemonList: daemonStruct
          | daemonStruct daemonList;

daemonStruct: tLBRAC dStructItems tRBRAC { metDaemon($2.ds); };

dStructItems: daemonItem
                {
		  if ($1.ds.name)
		    $$.ds.name = $1.ds.name;
		  else if ($1.ds.command)
		    $$.ds.command = $1.ds.command;
		  else if ($1.ds.host)
		    $$.ds.host = $1.ds.host;
		  else $$.ds.flavor = $1.ds.flavor;
		}
            | daemonItem dStructItems
                {
		  $$.ds.name = $2.ds.name;
		  $$.ds.command = $2.ds.command;
		  $$.ds.host = $2.ds.host;
		  $$.ds.flavor = $2.ds.flavor;
		  
		  if ($1.ds.name)
		    $$.ds.name = $1.ds.name;
		  else if ($1.ds.command)
		    $$.ds.command = $1.ds.command;
		  else if ($1.ds.host)
		    $$.ds.host = $1.ds.host;
		  else $$.ds.flavor = $1.ds.flavor;
		};

daemonItem: tNAME tLITERAL tSEMI
               { $$.ds.name = $2.cp;
		 $$.ds.command = 0; $$.ds.host = 0;}
           | tFLAVOR flavor tSEMI
               { $$.ds.flavor = $2.flav;
		 $$.ds.name = 0; $$.ds.host = 0; $$.ds.command = 0;}
           | tCOMMAND tLITERAL tSEMI
               { $$.ds.command = $2.cp;
		 $$.ds.host = 0; $$.ds.name = 0; }
           | tHOST tLITERAL tSEMI
               { $$.ds.host = $2.cp;
		 $$.ds.command = 0; $$.ds.name = 0; };

flavor: tPVM { $$.flav = metPVM;}
      | tCM5 { $$.flav = metCM5;};

