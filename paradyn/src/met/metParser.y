
%{ 
/*
 * Copyright (c) 1996 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

/*
 * $Log: metParser.y,v $
 * Revision 1.28  1997/06/07 21:01:25  newhall
 * replaced exclude_func and exclude_lib with exclude_node
 *
 * Revision 1.27  1997/06/05 04:29:46  newhall
 * added exclude_func mdl option to exclude shared object functions
 *
 * Revision 1.26  1997/05/02 18:23:41  mjrg
 * Removed use of class objects from struct parseStack which is allocated
 * with malloc
 *
 * Revision 1.25  1997/04/17 19:43:36  sec
 * Made is so that negative numbers are allowed.
 *
 * Revision 1.24  1997/04/14 20:03:53  zhichen
 * Added | tIDENT tLSQUARE tUNS tRSQUARE tDOT tIDENT for instr_rand.
 *
 * Revision 1.23  1997/03/29 02:07:03  sec
 * Added parsing commands to understand the wildcard * path in the matchPath
 * In addition added parsing commands to understand the new constraint
 * field, $constraint[0], etc. which is converted to $constraint0, $constraint1,
 * etc. and then translated in mdl.C to the value.
 *
 * Revision 1.22  1996/10/08 21:52:17  mjrg
 * changed the evaluation of resource lists
 * removed warnings
 *
 * Revision 1.21  1996/09/26 19:03:26  newhall
 * added "exclude_lib" mdl option
 *
 * Revision 1.20  1996/04/04  21:55:27  newhall
 * added limit option to visi definition
 *
 * Revision 1.19  1996/03/20  17:04:20  mjrg
 * Changed mdl to support calls with multiple arguments.
 *
 * Revision 1.18  1996/03/01 22:49:16  mjrg
 * Added type to resources.
 * Changes to the MDL to support the resource hierarchy.
 * Added unique names for lists, constraints and metrics.
 *
 * Revision 1.17  1996/01/05 20:01:27  newhall
 * fixed purify error
 *
 * Revision 1.16  1995/12/18 23:22:13  newhall
 * changed metric units type so that it can have one of 3 values (normalized,
 * unnormalized or sampled)
 *
 * Revision 1.15  1995/12/15 22:30:07  mjrg
 * Merged paradynd and paradyndPVM
 * Get module name for functions from symbol table in solaris
 * Fixed code generation for multiple instrumentation statements
 * Change syntax of MDL resource lists
 *
 * Revision 1.14  1995/11/28 15:52:57  naim
 * Minor fix. Changing char[number] by string - naim
 *
 * Revision 1.13  1995/11/21  21:07:11  naim
 * Fixing unitsType definition for MDL grammar - naim
 *
 * Revision 1.12  1995/11/21  15:15:37  naim
 * Changing the MDL grammar to allow more flexible metric definitions (i.e. we
 * can specify all elements in any order). Additionally, the option "fold"
 * has been removed - naim
 *
 * Revision 1.11  1995/11/17  17:22:13  newhall
 * added "unitsType" option to MDL, can be "normalized" or "unnormalized"
 *
 * Revision 1.10  1995/11/13  14:53:28  naim
 * Adding "mode" option to the Metric Description Language to allow specificacion
 * of developer mode for metrics (default mode is "normal") - naim
 *
 * Revision 1.9  1995/09/18  22:39:20  mjrg
 * Added directory command.
 * Removed host and user from daemon declaration.
 * Changed host and directory fields to string.
 *
 * Revision 1.8  1995/08/24  15:02:46  hollings
 * AIX/SP-2 port (including option for split instruction/data heaps)
 * Tracing of rexec (correctly spawns a paradynd if needed)
 * Added rtinst function to read getrusage stats (can now be used in metrics)
 * Critical Path
 * Improved Error reporting in MDL sematic checks
 * Fixed MDL Function call statement
 * Fixed bugs in TK usage (strings passed where UID expected)
 *
 * Revision 1.7  1995/06/02  20:49:02  newhall
 * made code compatable with new DM interface
 * fixed problem with force option in visiDef
 * fixed hpux link errors
 *
 * Revision 1.6  1995/05/26  21:51:35  markc
 * Changed tINT to tUNS.  The scanner does not produce tINT.
 *
 * Revision 1.5  1995/05/18  10:58:33  markc
 * mdl
 *
 * Revision 1.4  1995/02/16  08:24:21  markc
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
#include "util/h/hist.h"
#include <string.h>

#define YYSTYPE struct parseStack

extern int lineNo;
extern int yylex();
extern void *yyerror(const char *);
extern int yyparse();
extern void handle_error();
//extern bool hack_in_cons;
//extern void hack_cons_type(vector<string>*);
//extern unsigned hacked_cons_type;
%}

%token tDAEMON tPROCESS tTUNABLE_CONSTANT tIDENT 
%token tCOMMAND tARGS tHOST tLITERAL tFLOAT tCOMMA
%token tSEMI tFLAVOR tNAME
%token tRES_LIST tVISI tUSER tDIR tFALSE tTRUE tFORCE tLIMIT
%token tEXLIB

%token tT_PROCEDURE tT_MODULE tT_STRING tT_INT tT_FLOAT tTRUE tFALSE tDEFAULT
%token tFOREACH tLPAREN tRPAREN tLBLOCK tRBLOCK tCOLON tDOLLAR tAMPERSAND
%token tMETRIC tUNS tBASE tNAME tUNITS tBASE tIS tFUNCTION_CALL
%token tCOUNTER tAGG tAVG tSUM tMIN tMAX tDOT tW_TIME tP_TIME
%token tAPPEND tPREPEND tDERIVED tIF tREPLACE tCONSTRAINT tCONSTRAINED
%token tTYPE tAT tIN tLSQUARE tRSQUARE tBEFORE tAFTER
%token tSTYLE tEVENT_COUNTER tSAMPLE_FUNC tMODE tDEVELOPER tNORMAL
%token tUNITTYPE tNORMALIZE tUNNORMALIZE tSAMPLED
%token tPLUS tMINUS tDIV tMULT tLT tGT tLE tGE tEQ tNE tAND tOR tNOT
%token tADD_COUNTER tSET_COUNTER tSUB_COUNTER 
%token tSTART_PROC_TIMER tSTOP_PROC_TIMER 
%token tSTART_WALL_TIMER tSTOP_WALL_TIMER 
%token tRC tLC tASSIGN tPRE_INSN tPOST_INSN
%token tARG tRETURN
%token tREAD_SYMBOL tREAD_ADDRESS
%token tVOID tITEMS tLIBRARY
%left tLT tGT tEQ tNE tLE tGE
%left tPLUS tMINUS
%left tMULT tDIV
%left tRSQUARE tRPAREN
%right tLSQUARE tLPAREN

%%

stuff: { lineNo = 0; } definitions { yyclearin; return(0); };

definitions:
           | definition definitions;


definition: daemonDef
	  | processDef
	  | tunableConstant
          | resList
	  | visiDef
	  | exlibs
	  | error
          | metric_definition
          | ext_constraint_definition 
          | def_constraint_definition
          | metric_stmt {    mdl_data::stmts += $1.m_stmt; };

list_type: tT_STRING       { $$.u = MDL_T_STRING; }
         | tT_PROCEDURE    { $$.u = MDL_T_PROCEDURE_NAME; }
         | tT_MODULE       { $$.u = MDL_T_MODULE; }
         | tT_FLOAT        { $$.u = MDL_T_FLOAT; }
         | tT_INT          { $$.u = MDL_T_INT; };

opt_library: tCOMMA tTRUE     { $$.b = true; }
           | tCOMMA tFALSE    { $$.b = false; };

list_id: tCOMMA tLITERAL      { $$.sp = $2.sp; }

list_items: tCOMMA tLPAREN stringItems tRPAREN { $$.vs = $3.vs; }
          | tLBLOCK stringItems tRBLOCK { $$.vs = $2.vs; }

resList: tRES_LIST tLPAREN list_type list_id list_items opt_library tCOMMA tIDENT tRPAREN tSEMI { 
  vector<string> *temp = new vector<string>;
  *temp += *$8.sp;

  metParseError = ERR_NO_ERROR;
  T_dyninstRPC::mdl_stmt *s = new T_dyninstRPC::mdl_list_stmt($3.u, *$4.sp,
							      $5.vs, $6.b, temp);
  if (s) mdl_data::stmts += s;
  delete $4.sp; delete $8.sp;
}
 | tRES_LIST tIDENT tIS list_type tLBLOCK resListItems tRBLOCK 
       {
       T_dyninstRPC::mdl_stmt *s = new T_dyninstRPC::mdl_list_stmt($4.u, *$2.sp, 
								   $6.vs, $6.b, $6.vsf);
         mdl_data::unique_name(*$2.sp);
         if (s) mdl_data::stmts += s; 
         }
 | tRES_LIST error;

library : tTRUE { $$.b = true; }
        | tFALSE { $$.b = false; }

resListItems: 
             tITEMS list_items tSEMI met_flavor tLIBRARY library tSEMI
               { $$.vs = $2.vs; $$.vsf = $4.vs; $$.b = $6.b; }
           | tITEMS list_items tSEMI tLIBRARY library tSEMI met_flavor
               { $$.vs = $2.vs; $$.b = $5.b; $$.vsf = $7.vs; }
           | met_flavor tITEMS list_items tSEMI tLIBRARY library tSEMI
               { $$.vsf = $1.vs; $$.vs = $3.vs; $$.b = $6.b; }
	   | met_flavor tLIBRARY library tSEMI tITEMS list_items tSEMI
	       { $$.vsf = $1.vs; $$.b = $3.b; $$.vs = $6.vs; }
	   | tLIBRARY library tSEMI tITEMS list_items tSEMI met_flavor
	       { $$.b = $2.b; $$.vs = $5.vs; $$.vsf = $7.vs; }
	   | tLIBRARY library tSEMI met_flavor tITEMS list_items tSEMI
	       { $$.b = $2.b; $$.vsf = $4.vs; $$.vs = $6.vs; }
	   | tITEMS list_items tSEMI met_flavor 
               { $$.vs = $2.vs; $$.vsf = $4.vs; $$.b = false; }
	   | met_flavor tITEMS list_items tSEMI
	       { $$.vsf = $1.vs; $$.vs = $3.vs; $$.b = false; }

stringItems: tLITERAL {
                  $$.vs = new vector<string>; (*$$.vs) += *$1.sp; delete $1.sp; }
           | stringItems tCOMMA tLITERAL
                 {
		   $$.vs = $1.vs; (*$$.vs) += *$3.sp; delete $3.sp;
		 };

visiDef: tVISI tIDENT visiStruct {
             field f;
	     f.val = $2.sp; f.spec = SET_NAME;
             ($3.vm)->set_field(f);
	     delete $2.sp;
	     metParseError = ERR_NO_ERROR;
	     if (!visiMet::addVisi($3.vm))
	       handle_error();
           }
       | tVISI error;

visiStruct: tLBLOCK vItems tRBLOCK {$$.vm = $2.vm;};

vItems:      { $$.vm = new visiMet();}
       | vItems vItem
             { $$.vm = $1.vm; ($$.vm)->set_field($2.fld); delete $2.fld.val; };

vItem: tCOMMAND tLITERAL tSEMI
           { $$.fld.val = $2.sp; $$.fld.spec = SET_COMMAND;}
     | tHOST tLITERAL tSEMI
           { $$.fld.val = $2.sp; $$.fld.spec = SET_HOST;}
     | tUSER tIDENT tSEMI
           { $$.fld.val = $2.sp; $$.fld.spec = SET_USER;}
     | tDIR tLITERAL tSEMI
           { $$.fld.val = $2.sp; $$.fld.spec = SET_DIR;};
     | tFORCE tUNS tSEMI
           { $$.fld.force = $2.u; $$.fld.spec = SET_FORCE;};
     | tLIMIT tUNS tSEMI
           { $$.fld.limit = $2.u; $$.fld.spec = SET_LIMIT;};

processDef: tPROCESS tIDENT processItem {
               field f;
	       f.val = $2.sp; f.spec = SET_NAME;
	       ($3.pm)->set_field(f);
	       delete $2.sp;
	       metParseError = ERR_NO_ERROR;
	       if (!processMet::addProcess($3.pm)) {
	         handle_error();
	       }
             }
          | tPROCESS error;

processItem: tLBLOCK aItems tRBLOCK {$$.pm = $2.pm; };

aItems:      { $$.pm = new processMet();}
       |  aItems aItem
	     { $$.pm = $1.pm; ($$.pm)->set_field($2.fld); delete $2.fld.val;};

aItem: tCOMMAND tLITERAL tSEMI
           { $$.fld.val = $2.sp; $$.fld.spec = SET_COMMAND;}
     | tDAEMON tIDENT tSEMI
           { $$.fld.val = $2.sp; $$.fld.spec = SET_DAEMON;}
     | tHOST tLITERAL tSEMI
           { $$.fld.val = $2.sp; $$.fld.spec = SET_HOST;}
     | tUSER tIDENT tSEMI
           { $$.fld.val = $2.sp; $$.fld.spec = SET_USER;}
     | tDIR tLITERAL tSEMI
           { $$.fld.val = $2.sp; $$.fld.spec = SET_DIR;};


exlibs: tEXLIB exlibItem;

exlibItem: tLITERAL tSEMI {
		metParseError = ERR_NO_ERROR;
		mdl_data::lib_constraints += *$1.sp;
};

tunableConstant: tTUNABLE_CONSTANT tunableItem
               | tTUNABLE_CONSTANT tLBLOCK tunableList tRBLOCK
	       | tTUNABLE_CONSTANT tLBLOCK error tRBLOCK
	       | tTUNABLE_CONSTANT tLBLOCK error
	       | tTUNABLE_CONSTANT error;

tunableList: 
           | tunableList tunableItem;

tunableItem: tLITERAL tUNS tSEMI  {
                 metParseError = ERR_NO_ERROR;
                 if (!tunableMet::addTunable(*$1.sp, (float) $2.i))
		   handle_error();
	       }
           | tLITERAL tFLOAT tSEMI {
	         metParseError = ERR_NO_ERROR;
	         if (!tunableMet::addTunable(*$1.sp, $2.f))
                   handle_error();
	       }
           | tLITERAL tTRUE tSEMI {
                 metParseError = ERR_NO_ERROR;
                 if (!tunableMet::addTunable(*$1.sp, true))
		   handle_error();
	       }
           | tLITERAL tFALSE tSEMI {
                 metParseError = ERR_NO_ERROR;
                 if (!tunableMet::addTunable(*$1.sp, false))
		   handle_error();
	       };

daemonDef: tDAEMON tIDENT daemonStruct {
             field f;
	     f.val = $2.sp; f.spec = SET_NAME;
	     ($3.dm)->set_field(f);
	     delete $2.sp;
	     metParseError = ERR_NO_ERROR;
	     if (!daemonMet::addDaemon($3.dm)) {
	       handle_error();
	     }
	   }
	 | tDAEMON error;

daemonStruct: tLBLOCK dStructItems tRBLOCK {$$.dm = $2.dm;};

dStructItems:    { $$.dm = new daemonMet();}
            |  dStructItems daemonItem
                 { $$.dm = $1.dm; ($$.dm)->set_field($2.fld);
		   delete $2.fld.val; delete $2.fld.flav; };

daemonItem:  tCOMMAND tLITERAL tSEMI
               { $$.fld.val = $2.sp; $$.fld.spec = SET_COMMAND;}
           | tUSER tIDENT tSEMI
               { $$.fld.val = $2.sp; $$.fld.spec = SET_USER;}
           | tFLAVOR tIDENT tSEMI
               { $$.fld.flav = $2.sp; $$.fld.spec = SET_FLAVOR;}
           | tHOST tLITERAL tSEMI
               { $$.fld.val = $2.sp; $$.fld.spec = SET_HOST;}
           | tDIR tIDENT tSEMI
              { $$.fld.val = $2.sp; $$.fld.spec = SET_DIR;};

met_name: tNAME tLITERAL tSEMI { $$.sp = $2.sp; };

met_units: tUNITS tIDENT tSEMI { $$.sp = $2.sp; };

agg_val: tAVG { $$.u = MDL_AGG_AVG; }
      | tSUM { $$.u = MDL_AGG_SUM; }
      | tMIN { $$.u = MDL_AGG_MIN; }
      | tMAX { $$.u = MDL_AGG_MAX; };

met_agg: tAGG agg_val tSEMI { $$.u = $2.u;};

met_style: tSTYLE tEVENT_COUNTER tSEMI { $$.u = EventCounter; }
         | tSTYLE tSAMPLE_FUNC tSEMI   { $$.u = SampledFunction; }

bin_op: tPLUS { $$.u = MDL_PLUS; }
     | tMINUS { $$.u = MDL_MINUS; }
     | tDIV { $$.u = MDL_DIV; }
     | tMULT { $$.u = MDL_MULT; }
     | tLT { $$.u = MDL_LT; }
     | tGT { $$.u = MDL_GT; }
     | tLE { $$.u = MDL_LE; }
     | tGE { $$.u = MDL_GE; }
     | tEQ { $$.u = MDL_EQ; }
     | tNE { $$.u = MDL_NE; }
     | tAND { $$.u = MDL_AND; }
     | tOR { $$.u = MDL_OR; };

position: tAPPEND { $$.u = MDL_APPEND; }
        | tPREPEND { $$.u = MDL_PREPEND; };

instr_expr: instr_rand {
  $$.expr.rand1 = $1.rand; $$.expr.bin_op = MDL_T_NONE;
} | instr_rand instr_op instr_rand {
  $$.expr.rand1 = $1.rand; $$.expr.rand2 = $3.rand; $$.expr.bin_op = $2.u;
};

instr_op: tLT       { $$.u = MDL_LT; }
        | tGT       { $$.u = MDL_GT; }
        | tLE       { $$.u = MDL_LE; }
        | tGE       { $$.u = MDL_GE; }
        | tEQ       { $$.u = MDL_EQ; }
        | tNE       { $$.u = MDL_NE; };

instr_rands: instr_rands tCOMMA instr_rand {
  $$.pars = $1.pars;
  (*($$.pars)) += $3.rand;
} | instr_rand {
  $$.pars = new vector<T_dyninstRPC::mdl_instr_rand *>;
  (*($$.pars)) += $1.rand;
};

instr_rand: tIDENT                       { $$.rand = new T_dyninstRPC::mdl_instr_rand(MDL_T_COUNTER, *$1.sp); }
          | tAMPERSAND tIDENT            { $$.rand = new T_dyninstRPC::mdl_instr_rand(MDL_T_COUNTER_PTR, *$2.sp); }
          | tUNS                         { $$.rand = new T_dyninstRPC::mdl_instr_rand(MDL_T_INT, $1.u); }
          | tMINUS tUNS                  { $$.rand = new T_dyninstRPC::mdl_instr_rand(MDL_T_INT, -$1.u); }
          | tARG tLSQUARE tUNS tRSQUARE  { $$.rand = new T_dyninstRPC::mdl_instr_rand(MDL_ARG, $3.u); }
          | tRETURN                      { $$.rand = new T_dyninstRPC::mdl_instr_rand(MDL_RETURN); }
          | tREAD_SYMBOL tLPAREN tLITERAL tRPAREN {
	                                   $$.rand = new T_dyninstRPC::mdl_instr_rand(MDL_READ_SYMBOL, *$3.sp); }
          | tREAD_ADDRESS tLPAREN tUNS tRPAREN {
	                                   $$.rand = new T_dyninstRPC::mdl_instr_rand(MDL_READ_ADDRESS, $3.u); }
          | tFUNCTION_CALL tLPAREN tLITERAL tRPAREN {
	                                   $$.rand = new T_dyninstRPC::mdl_instr_rand(MDL_CALL_FUNC, *$3.sp); }
          | tFUNCTION_CALL tLPAREN tLITERAL tCOMMA instr_rands tRPAREN {
	                                   $$.rand = new T_dyninstRPC::mdl_instr_rand(MDL_CALL_FUNC, *$3.sp, *$5.pars); }
          | tIDENT tLPAREN instr_rands tRPAREN {
	                                   $$.rand = new T_dyninstRPC::mdl_instr_rand(MDL_CALL_FUNC, *$1.sp, *$3.pars); }
          | tIDENT tLPAREN tRPAREN {
                                           $$.rand = new T_dyninstRPC::mdl_instr_rand(MDL_CALL_FUNC, *$1.sp); }
	  | tIDENT tLSQUARE tUNS tRSQUARE tDOT tIDENT {//Blizzard
             if(*$1.sp == "$constraint") {
                 $$.rand = new T_dyninstRPC::mdl_instr_rand(MDL_T_RECORD, *$6.sp) ;
             } else
                 yyerror("only $constraint is allowed\n");
	  }
          | tIDENT tLSQUARE tUNS tRSQUARE {
	      if (*$1.sp == "$constraint") {
                $$.rand = new T_dyninstRPC::mdl_instr_rand(MDL_T_INT, string("$constraint") + string($3.u));
              }
            };


instr_req: tSET_COUNTER tLPAREN tIDENT tCOMMA instr_rand tRPAREN tSEMI {
  $$.instr_req = new T_dyninstRPC::mdl_instr_req($5.rand, MDL_SET_COUNTER, *$3.sp);
  delete $3.sp;
} | tADD_COUNTER tLPAREN tIDENT tCOMMA instr_rand tRPAREN tSEMI {
  $$.instr_req = new T_dyninstRPC::mdl_instr_req($5.rand, MDL_ADD_COUNTER, *$3.sp);
  delete $3.sp;
} | tSUB_COUNTER tLPAREN tIDENT tCOMMA instr_rand tRPAREN tSEMI {
  $$.instr_req = new T_dyninstRPC::mdl_instr_req($5.rand, MDL_SUB_COUNTER, *$3.sp);
  delete $3.sp;
} | tSTART_PROC_TIMER tLPAREN tIDENT tRPAREN tSEMI { 
  $$.instr_req = new T_dyninstRPC::mdl_instr_req(MDL_START_PROC_TIMER, *$3.sp);
  delete $3.sp; 
} | tSTOP_PROC_TIMER tLPAREN tIDENT tRPAREN tSEMI {
  $$.instr_req = new T_dyninstRPC::mdl_instr_req(MDL_STOP_PROC_TIMER, *$3.sp);
  delete $3.sp; 
} | tSTART_WALL_TIMER tLPAREN tIDENT tRPAREN tSEMI {
  $$.instr_req = new T_dyninstRPC::mdl_instr_req(MDL_START_WALL_TIMER, *$3.sp);
  delete $3.sp; 
} | tSTOP_WALL_TIMER tLPAREN tIDENT tRPAREN tSEMI {
  $$.instr_req = new T_dyninstRPC::mdl_instr_req(MDL_STOP_WALL_TIMER, *$3.sp);
  delete $3.sp; 
} | instr_rand tSEMI {
  if ($1.rand->type_ == MDL_CALL_FUNC)
    $$.instr_req = new T_dyninstRPC::mdl_instr_req(MDL_CALL_FUNC, $1.rand);
  else
    yyerror("Illegal instrumentation request");
}    


instr_code: tIF tLPAREN instr_expr tRPAREN instr_req {
  $$.i_code = new T_dyninstRPC::mdl_icode($3.expr.rand1, $3.expr.rand2,
					  $3.expr.bin_op, true, $5.instr_req);
} | instr_req {
  $$.i_code = new T_dyninstRPC::mdl_icode(0, 0, 0, false, $1.instr_req);
};

instr_code_list:                   { $$.icode_v = new vector<T_dyninstRPC::mdl_icode*>; }
               | instr_code_list instr_code {
		                                $$.icode_v = $1.icode_v;
						(*$$.icode_v) += $2.i_code;
					      };

opt_constrained:              { $$.b = false;}
               | tCONSTRAINED { $$.b = true;};

fields:                    { $$.vs = new vector<string>; }
      | tDOT tIDENT fields { $$.vs = $3.vs; (*$$.vs) += *$2.sp; delete $2.sp; };

arg_list:                      { $$.m_expr_v = new vector<T_dyninstRPC::mdl_expr*>; }
        | metric_expr          { $$.m_expr_v = new vector<T_dyninstRPC::mdl_expr*>;
			         (*$$.m_expr_v) += $1.m_expr; }
        | arg_list tCOMMA metric_expr   { $$.m_expr_v = $1.m_expr_v;
					  (*$$.m_expr_v) += $3.m_expr; };

variable: tIDENT           { $$.sp = $1.sp; };

metric_expr: variable fields {
          $$.m_expr = new T_dyninstRPC::mdl_v_expr(*$1.sp, *$2.vs);
	  delete $2.vs; delete $1.sp;
} | tIDENT tLPAREN arg_list tRPAREN {
          $$.m_expr = new T_dyninstRPC::mdl_v_expr(*$1.sp, $3.m_expr_v); delete $1.sp;
} | tUNS {
          $$.m_expr = new T_dyninstRPC::mdl_v_expr($1.u);
} | tMINUS tUNS {
          $$.m_expr = new T_dyninstRPC::mdl_v_expr(-$1.u);
} | tLITERAL {
          $$.m_expr = new T_dyninstRPC::mdl_v_expr(*$1.sp); delete $1.sp;
} | metric_expr bin_op metric_expr {
          $$.m_expr = new T_dyninstRPC::mdl_v_expr($2.u, $1.m_expr, $3.m_expr);
} | tIDENT tLSQUARE tUNS tRSQUARE {
          $$.m_expr = new T_dyninstRPC::mdl_v_expr(*$1.sp, $3.u); delete $1.sp;
} | tIDENT tLSQUARE tUNS tRSQUARE fields {
	if (*$1.sp == "$constraint") {
	   $$.m_expr = new T_dyninstRPC::mdl_v_expr(string("$constraint") + string($3.u), *$5.vs);
	   delete $5.vs;
	   delete $1.sp;
	} else {
	   yyerror("Syntax error");
           exit(-1);
        }
} | tLPAREN metric_expr tRPAREN {
          $$.m_expr = $2.m_expr;
}; 

where_instr: tPRE_INSN   { $$.u = MDL_PRE_INSN; }
           | tPOST_INSN  { $$.u = MDL_POST_INSN; }

instr_request: position where_instr metric_expr opt_constrained tLC instr_code_list tRC {
  $$.m_stmt = new T_dyninstRPC::mdl_instr_stmt($1.u, $3.m_expr, $6.icode_v, $2.u, $4.b);
};


metric_stmts:                   { $$.m_stmt_v = new vector<T_dyninstRPC::mdl_stmt*>;}
            | metric_stmts metric_stmt {
	                        $$.m_stmt_v = $1.m_stmt_v; 
			        (*$$.m_stmt_v) += $2.m_stmt;}


metric_stmt:  tLBLOCK metric_stmts tRBLOCK {
                           $$.m_stmt = new T_dyninstRPC::mdl_seq_stmt($2.m_stmt_v); }
           | tFOREACH tIDENT tIN metric_expr metric_stmt {
	                   $$.m_stmt = new T_dyninstRPC::mdl_for_stmt(*$2.sp, $4.m_expr, $5.m_stmt);
			   delete $2.sp; }
           | tIF metric_expr metric_stmt {
	                   $$.m_stmt = new T_dyninstRPC::mdl_if_stmt($2.m_expr, $3.m_stmt); }
           | instr_request {
	                   $$.m_stmt = $1.m_stmt;
			 };

flavor_list: tIDENT {
                       $$.vs = new vector<string>; (*$$.vs) += *$1.sp; delete $1.sp; }
           | flavor_list tCOMMA tIDENT {
                       $$.vs = $1.vs; (*$$.vs) += *$3.sp; delete $3.sp; };

met_flavor: tFLAVOR tASSIGN tLBLOCK flavor_list tRBLOCK tSEMI {$$.vs = $4.vs;};
	| tFLAVOR tLBLOCK flavor_list tRBLOCK tSEMI { $$.vs = $3.vs;};


mode_val: tDEVELOPER { $$.b = true; }
	| tNORMAL { $$.b = false; }; 

met_mode: tMODE mode_val tSEMI { $$.b = $2.b; };

unittype_val: tNORMALIZE { $$.i = 1; }
	| tUNNORMALIZE { $$.i = 0; }
	| tSAMPLED { $$.i = 2; };

met_unittype: tUNITTYPE unittype_val tSEMI {$$.i = $2.i;};	

met_base: tBASE tIS tCOUNTER tLBLOCK metric_stmts tRBLOCK {
  $$.base.type = MDL_T_COUNTER;
  $$.base.m_stmt_v = $5.m_stmt_v;
} | tBASE tIS tP_TIME tLBLOCK metric_stmts tRBLOCK { 
  $$.base.type = MDL_T_PROC_TIMER;
  $$.base.m_stmt_v = $5.m_stmt_v; 
} | tBASE tIS tW_TIME tLBLOCK metric_stmts tRBLOCK { 
  $$.base.type = MDL_T_WALL_TIMER;
  $$.base.m_stmt_v = $5.m_stmt_v; 
} | tBASE tIS tVOID tLBLOCK metric_stmts tRBLOCK {
  // for special un-sampled metrics.
  $$.base.type = MDL_T_NONE;
  $$.base.m_stmt_v = $5.m_stmt_v; 
};

constraint_list: tCONSTRAINT tIDENT tSEMI {
		 T_dyninstRPC::mdl_constraint *c =
					      mdl_data::new_constraint(*$2.sp,
						            NULL, NULL, false,
							    MDL_T_COUNTER);
	         delete $2.sp;
		 $$.mfld = new metricFld;
		 $$.mfld->constraint = c; }

	       | int_constraint_definition {
		 $$.mfld = new metricFld;
	         $$.mfld->constraint = $1.constraint; };

met_temps: tCOUNTER tIDENT tSEMI { $$.sp = $2.sp; };

metric_struct: tLBLOCK metric_list tRBLOCK {$$.mde = $2.mde;};

metric_list: {$$.mde = new metricDef();}
	   | metric_list metric_elem_list { $$.mde = $1.mde;
				            $$.mde->setField(*$2.mfld);
					    delete $2.mfld };

metric_elem_list: met_name {$$.mfld = new metricFld;
			    $$.mfld->name = *$1.sp; $$.mfld->spec = SET_MNAME;
                            delete $1.sp;}
                | met_units {$$.mfld = new metricFld;
			     $$.mfld->units = *$1.sp; $$.mfld->spec = SET_UNITS;
			     delete $1.sp;}
                | met_agg {$$.mfld = new metricFld;
			   $$.mfld->agg = $1.u; $$.mfld->spec = SET_AGG;}
                | met_style {$$.mfld = new metricFld;
			     $$.mfld->style = $1.u; $$.mfld->spec = SET_STYLE;}
                | met_flavor {$$.mfld = new metricFld;
			      $$.mfld->flavor = $1.vs;
                              $$.mfld->spec = SET_MFLAVOR;}
                | met_mode {$$.mfld = new metricFld;
			    $$.mfld->mode = $1.b; $$.mfld->spec = SET_MODE;}
                | constraint_list {$$.mfld = new metricFld;
				   $$.mfld->constraint = $1.mfld->constraint;
                                   $$.mfld->spec = SET_CONSTRAINT;
				   delete $1.mfld;}
                | met_temps {$$.mfld = new metricFld;
			     $$.mfld->temps = *$1.sp;
                             $$.mfld->spec = SET_TEMPS;}
                | met_base {$$.mfld = new metricFld;
			    $$.mfld->base_type = $1.base.type;
                            $$.mfld->base_m_stmt_v = $1.base.m_stmt_v;
                            $$.mfld->spec = SET_BASE;}
		| met_unittype { $$.mfld = new metricFld;
				 $$.mfld->unittype = $1.i; 
				 $$.mfld->spec = SET_UNITTYPE; };

metric_definition: tMETRIC tIDENT metric_struct {
  if (!($3.mde->addNewMetricDef(*$2.sp)))
  {
    string msg;
    msg = string("Error defining ") + (*$2.sp);
    yyerror(msg.string_of());
    msg = $3.mde->missingFields();
    yyerror(msg.string_of());
  }
  delete $2.sp;
  delete $3.mde;
};

match_path:                        {$$.vs = new vector<string>; }
          | match_path tDIV tIDENT {
	                $$.vs = $1.vs; (*$$.vs) += *$3.sp; delete $3.sp;
			///hack_cons_type($$.vs); 
		      }
          | match_path tDIV tMULT {
		        $$.vs = $1.vs; (*$$.vs) += "*";
                      };

def_constraint_definition: tCONSTRAINT tIDENT match_path tIS tDEFAULT tSEMI {
  T_dyninstRPC::mdl_constraint *c = mdl_data::new_constraint(*$2.sp, $3.vs, NULL, false,
							     MDL_T_NONE);
  if (!c) {
    string msg = string("Error, did not new mdl_constraint\n");
    yyerror(P_strdup(msg.string_of()));
    exit(-1);
  } else {
    mdl_data::unique_name(*$2.sp);
    mdl_data::all_constraints += c;
    delete $2.sp; 
  }
}

ext_constraint_definition: tCONSTRAINT tIDENT match_path tIS tCOUNTER tLBLOCK metric_stmts tRBLOCK {
  //hack_in_cons = false;
  T_dyninstRPC::mdl_constraint *c = mdl_data::new_constraint(*$2.sp, $3.vs,
							     $7.m_stmt_v, false,
							     MDL_T_COUNTER);
  if (!c) {
    yyerror("Error, did not new mdl_constraint\n");
    exit(-1);
  } else {
    mdl_data::unique_name(*$2.sp);
    mdl_data::all_constraints += c;
    delete $2.sp; 
  }
};

cons_type: tCOUNTER  { $$.u = MDL_T_COUNTER; }
         | tW_TIME    { $$.u = MDL_T_WALL_TIMER; }
         | tP_TIME    { $$.u = MDL_T_PROC_TIMER; };

int_constraint_definition: tCONSTRAINT tIDENT match_path tIS tREPLACE cons_type tLBLOCK metric_stmts tRBLOCK {
  $$.constraint = mdl_data::new_constraint(*$2.sp, $3.vs, $8.m_stmt_v, true, $6.u);
  if (!$$.constraint) {
    string msg = string("Error, did not new mdl_constraint\n");
    yyerror(P_strdup(msg.string_of()));
    exit(-1);
  }
  delete $2.sp; 
};


