
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

// $Id: metParser.y,v 1.36 2001/04/25 18:42:26 wxd Exp $

#include "paradyn/src/met/metParse.h"
#include "pdutilOld/h/hist.h"
#include <string.h>

#define YYSTYPE struct parseStack

extern int lineNo;
extern int yylex();
extern void yyerror(const char *);
extern int yyparse();
extern void handle_error();
%}

%token tDAEMON tPROCESS tTUNABLE_CONSTANT tIDENT 
%token tCOMMAND tHOST tLITERAL tFLOAT tCOMMA
%token tREMOTE_SHELL tAUTO_START
%token tSEMI tFLAVOR tNAME
%token tRES_LIST tVISI tUSER tDIR tFALSE tTRUE tFORCE tLIMIT tMETFOCUS
%token tEXLIB tNOCASE tREGEX
%token tT_PROCEDURE tT_MODULE tT_STRING tT_INT tT_FLOAT tTRUE tFALSE tDEFAULT
%token tFOREACH tLPAREN tRPAREN tLBLOCK tRBLOCK tDOLLAR tAMPERSAND
%token tMETRIC tUNS tBASE tUNITS tIS tFUNCTION_CALL
%token tCOUNTER tAGG tAVG tSUM tMIN tMAX tDOT tW_TIME tP_TIME
%token tAPPEND tPREPEND tDERIVED tIF tREPLACE tCONSTRAINT tCONSTRAINED
%token tIN tLSQUARE tRSQUARE
%token tSTYLE tEVENT_COUNTER tSAMPLE_FUNC tMODE tDEVELOPER tNORMAL
%token tUNITTYPE tNORMALIZE tUNNORMALIZE tSAMPLED

%token tPLUS tMINUS tDIV tMULT tLT tGT tLE tGE tEQ tNE tAND tOR
%token tRC tLC tASSIGN tPRE_INSN tPOST_INSN
%token tRETURN tARG
%token tVOID tITEMS tLIBRARY
%right tASSIGN tPLUSASSIGN tMINUSASSIGN
%left tLT tGT tEQ tNE tLE tGE
%left tPLUS tMINUS
%left tMULT tDIV
%right tPLUSPLUS
%left tNEG
%left tADDRESS
%left tRSQUARE tRPAREN
%right tLSQUARE tLPAREN

%%

stuff: { lineNo = 0; } definitions { yyclearin; return(0); }
  ;

definitions:
  | definitions definition
  ;

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
  | metric_stmt { mdl_data::stmts += $1.m_stmt; }
  ;

list_type: tT_STRING       { $$.u = MDL_T_STRING; }
  | tT_PROCEDURE    { $$.u = MDL_T_PROCEDURE_NAME; }
  | tT_MODULE       { $$.u = MDL_T_MODULE; }
  | tT_FLOAT        { $$.u = MDL_T_FLOAT; }
  | tT_INT          { $$.u = MDL_T_INT; }
  ;

opt_library: tCOMMA tTRUE     { $$.b = true; }
  | tCOMMA tFALSE    { $$.b = false; }
  ;

list_id: tCOMMA tLITERAL      { $$.sp = $2.sp; }
  ;

list_items: tCOMMA tLPAREN stringItems tRPAREN { $$.vs = $3.vs; }
  | tLBLOCK stringItems tRBLOCK { $$.vs = $2.vs; }
  ;

resList: tRES_LIST tLPAREN list_type list_id list_items opt_library tCOMMA tIDENT tRPAREN tSEMI
    { 
        vector<string> *temp = new vector<string>;
        *temp += *$8.sp;

        metParseError = ERR_NO_ERROR;
        T_dyninstRPC::mdl_stmt *s = new T_dyninstRPC::mdl_list_stmt(
            $3.u, *$4.sp, $5.vs, $6.b, temp);
        if (s) mdl_data::stmts += s;
        delete $4.sp; delete $8.sp;
    }
    | tRES_LIST tIDENT tIS list_type tLBLOCK resListItems tRBLOCK 
    {
        T_dyninstRPC::mdl_stmt *s = new T_dyninstRPC::mdl_list_stmt(
            $4.u, *$2.sp, $6.vs, $6.b, $6.vsf);
        mdl_data::unique_name(*$2.sp);
        if (s) mdl_data::stmts += s; 
    }
    | tRES_LIST error
    ;

library : tTRUE { $$.b = true; }
        | tFALSE { $$.b = false; }
	;

resListItems: tITEMS list_items tSEMI met_flavor tLIBRARY library tSEMI
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
	;

stringItems: tLITERAL 
		{ $$.vs = new vector<string>; (*$$.vs) += *$1.sp; delete $1.sp; }
	| stringItems tCOMMA tLITERAL 
		{ $$.vs = $1.vs; (*$$.vs) += *$3.sp; delete $3.sp; }
	;

visiDef: tVISI tIDENT visiStruct 
    {
        field f;
        f.val = $2.sp; f.spec = SET_NAME;
        ($3.vm)->set_field(f);
        delete $2.sp;
        metParseError = ERR_NO_ERROR;
        if (!visiMet::addVisi($3.vm))
            handle_error();
    }
    | tVISI error
    ;

visiStruct: tLBLOCK vItems tRBLOCK {$$.vm = $2.vm;}
	;

vItems: { $$.vm = new visiMet();}
	| vItems vItem
		{ $$.vm = $1.vm; ($$.vm)->set_field($2.fld); delete $2.fld.val; }
	;

vItem: tCOMMAND tLITERAL tSEMI
           { $$.fld.val = $2.sp; $$.fld.spec = SET_COMMAND;}
     | tHOST tLITERAL tSEMI
           { $$.fld.val = $2.sp; $$.fld.spec = SET_HOST;}
     | tUSER tIDENT tSEMI
           { $$.fld.val = $2.sp; $$.fld.spec = SET_USER;}
     | tDIR tLITERAL tSEMI
           { $$.fld.val = $2.sp; $$.fld.spec = SET_DIR;}
     | tFORCE tUNS tSEMI
           { $$.fld.force = $2.u; $$.fld.spec = SET_FORCE;}
     | tLIMIT tUNS tSEMI
           { $$.fld.limit = $2.u; $$.fld.spec = SET_LIMIT;}
     | tMETFOCUS tLITERAL tSEMI
           { $$.fld.val = $2.sp; $$.fld.spec = SET_METFOCUS;}
	;

processDef: tPROCESS tIDENT processItem 
	{
		field f;
		f.val = $2.sp; f.spec = SET_NAME;
		($3.pm)->set_field(f);
		delete $2.sp;
		metParseError = ERR_NO_ERROR;
		if (!processMet::addProcess($3.pm))
			handle_error();
	}
	| tPROCESS error
	;

processItem: tLBLOCK aItems tRBLOCK {$$.pm = $2.pm; }
	;

aItems:      { $$.pm = new processMet();}
	| aItems aItem
		{ $$.pm = $1.pm; ($$.pm)->set_field($2.fld); delete $2.fld.val;}
	;

aItem: tCOMMAND tLITERAL tSEMI
           { $$.fld.val = $2.sp; $$.fld.spec = SET_COMMAND;}
	| tDAEMON tIDENT tSEMI
           { $$.fld.val = $2.sp; $$.fld.spec = SET_DAEMON;}
	| tHOST tLITERAL tSEMI
           { $$.fld.val = $2.sp; $$.fld.spec = SET_HOST;}
	| tUSER tIDENT tSEMI
           { $$.fld.val = $2.sp; $$.fld.spec = SET_USER;}
	| tDIR tLITERAL tSEMI
           { $$.fld.val = $2.sp; $$.fld.spec = SET_DIR;}
    | tAUTO_START tTRUE tSEMI
           { $$.fld.bval = true; $$.fld.spec = SET_AUTO_START;}
    | tAUTO_START tFALSE tSEMI
           { $$.fld.bval = false; $$.fld.spec = SET_AUTO_START;}
	;

exlibs: tEXLIB exlibItem
	;

exlibItem: tNOCASE tLITERAL tSEMI
	    {
		metParseError = ERR_NO_ERROR;
		mdl_data::lib_constraints += *$2.sp;
		mdl_data::lib_constraint_flags += LIB_CONSTRAINT_NOCASE_FLAG;
	    }
	| tREGEX tLITERAL tSEMI
	    {
		metParseError = ERR_NO_ERROR;
		mdl_data::lib_constraints += *$2.sp;
		mdl_data::lib_constraint_flags += LIB_CONSTRAINT_REGEX_FLAG;
	    }
	| tNOCASE tREGEX tLITERAL tSEMI
	    {
		metParseError = ERR_NO_ERROR;
		mdl_data::lib_constraints += *$3.sp;
		mdl_data::lib_constraint_flags += 
		    LIB_CONSTRAINT_NOCASE_FLAG | LIB_CONSTRAINT_REGEX_FLAG;
	    }
	| tREGEX tNOCASE tLITERAL tSEMI
	    {
		metParseError = ERR_NO_ERROR;
		mdl_data::lib_constraints += *$3.sp;
		mdl_data::lib_constraint_flags += 
		    LIB_CONSTRAINT_NOCASE_FLAG | LIB_CONSTRAINT_REGEX_FLAG;
	    }
	| tLITERAL tSEMI 
	    {
		metParseError = ERR_NO_ERROR;
		mdl_data::lib_constraints += *$1.sp;
		mdl_data::lib_constraint_flags += 0;
	    }
	;

tunableConstant: tTUNABLE_CONSTANT tunableItem
	| tTUNABLE_CONSTANT tLBLOCK tunableList tRBLOCK
	| tTUNABLE_CONSTANT tLBLOCK error tRBLOCK
	| tTUNABLE_CONSTANT tLBLOCK error
	| tTUNABLE_CONSTANT error
	;

tunableList: tunableItem
	| tunableList tunableItem
	;

tunableItem: tLITERAL tUNS tSEMI  
	{
		metParseError = ERR_NO_ERROR;
		if (!tunableMet::addTunable(*$1.sp, (float) $2.i))
			handle_error();
	}
	| tLITERAL tFLOAT tSEMI 
	{
		metParseError = ERR_NO_ERROR;
		if (!tunableMet::addTunable(*$1.sp, $2.f))
			handle_error();
	}
	| tLITERAL tTRUE tSEMI 
	{
		metParseError = ERR_NO_ERROR;
		if (!tunableMet::addTunable(*$1.sp, true))
			handle_error();
	}
	| tLITERAL tFALSE tSEMI 
	{
		metParseError = ERR_NO_ERROR;
		if (!tunableMet::addTunable(*$1.sp, false))
			handle_error();
	}
	;

daemonDef: tDAEMON tIDENT daemonStruct 
	{
		field f;
		f.val = $2.sp; f.spec = SET_NAME;
		($3.dm)->set_field(f);
		delete $2.sp;
		metParseError = ERR_NO_ERROR;
		if (!daemonMet::addDaemon($3.dm)) 
			handle_error();
	}
	| tDAEMON error
	;

daemonStruct: tLBLOCK dStructItems tRBLOCK {$$.dm = $2.dm;}
	;

dStructItems:    { $$.dm = new daemonMet();}
	| dStructItems daemonItem 
	{ 
		$$.dm = $1.dm; ($$.dm)->set_field($2.fld);
		delete $2.fld.val; delete $2.fld.flav; 
	}
	;

daemonItem:  tCOMMAND tLITERAL tSEMI
		{ $$.fld.val = $2.sp; $$.fld.spec = SET_COMMAND;}
    | tREMOTE_SHELL tLITERAL tSEMI { $$.fld.val = $2.sp; $$.fld.spec = SET_REMSH;}
	| tUSER tIDENT tSEMI { $$.fld.val = $2.sp; $$.fld.spec = SET_USER;}
	| tFLAVOR tIDENT tSEMI { $$.fld.flav = $2.sp; $$.fld.spec = SET_FLAVOR;}
	| tHOST tLITERAL tSEMI { $$.fld.val = $2.sp; $$.fld.spec = SET_HOST;}
	| tDIR tIDENT tSEMI { $$.fld.val = $2.sp; $$.fld.spec = SET_DIR;}
	;

met_name: tNAME tLITERAL tSEMI { $$.sp = $2.sp; }
	;

met_units: tUNITS tIDENT tSEMI { $$.sp = $2.sp; }
	;

agg_val: tAVG { $$.u = MDL_AGG_AVG; }
      | tSUM { $$.u = MDL_AGG_SUM; }
      | tMIN { $$.u = MDL_AGG_MIN; }
      | tMAX { $$.u = MDL_AGG_MAX; }
	;

met_agg: tAGG agg_val tSEMI { $$.u = $2.u;}
	;

met_style: tSTYLE tEVENT_COUNTER tSEMI { $$.u = EventCounter; }
	| tSTYLE tSAMPLE_FUNC tSEMI   { $$.u = SampledFunction; }
	;

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
  | tOR { $$.u = MDL_OR; }
  ;

u_op: tPLUSPLUS { $$.u = MDL_PLUSPLUS; }
  ;

assign_op: tASSIGN { $$.u = MDL_ASSIGN; }
  | tPLUSASSIGN { $$.u = MDL_PLUSASSIGN; }
  | tMINUSASSIGN { $$.u = MDL_MINUSASSIGN; }
  ;

position: tAPPEND { $$.u = MDL_APPEND; }
	| tPREPEND { $$.u = MDL_PREPEND; }
	;

instr_code: tIF tLPAREN metric_expr tRPAREN metric_expr tSEMI
	{
		$$.i_code = new T_dyninstRPC::mdl_icode ($3.m_expr, $5.m_expr);
	}
	| metric_expr tSEMI
	{ 
		$$.i_code = new T_dyninstRPC::mdl_icode(0, $1.m_expr);
	}
	;

instr_code_list: { $$.icode_v = new vector<T_dyninstRPC::mdl_icode*>; }
  | instr_code_list instr_code 
    { $$.icode_v = $1.icode_v; (*$$.icode_v) += $2.i_code; }
  ;

opt_constrained: { $$.b = false;}
  | tCONSTRAINED { $$.b = true;}
  ;

fields: tIDENT 
  { $$.vs = new vector<string>; (*$$.vs) += *$1.sp; delete $1.sp; }
  | fields tDOT tIDENT { $$.vs = $1.vs; (*$$.vs) += *$3.sp; delete $3.sp; }
  ;

metric_expr_list: { $$.m_expr_v = new vector<T_dyninstRPC::mdl_expr*>; }
  | metric_expr 
  {
    $$.m_expr_v = new vector<T_dyninstRPC::mdl_expr*>;
    (*$$.m_expr_v) += $1.m_expr; 
  }
  | metric_expr_list tCOMMA metric_expr 
    { $$.m_expr_v = $1.m_expr_v; (*$$.m_expr_v) += $3.m_expr; }
  ;

metric_expr: tUNS { $$.m_expr = new T_dyninstRPC::mdl_v_expr($1.u); }
  | tLITERAL 
  {
    $$.m_expr = new T_dyninstRPC::mdl_v_expr(*$1.sp, true); delete $1.sp;
  }
  | tRETURN
  {
    $$.m_expr = new T_dyninstRPC::mdl_v_expr ("$return", false);
  }
  | tIDENT
  {
    $$.m_expr = new T_dyninstRPC::mdl_v_expr (*$1.sp, false); delete $1.sp;
  }
  | tIDENT assign_op metric_expr
  {
    $$.m_expr = new T_dyninstRPC::mdl_v_expr (*$1.sp, $2.u, $3.m_expr);
    delete $1.sp;
  }
  | tAMPERSAND metric_expr %prec tADDRESS
  {
    $$.m_expr = new T_dyninstRPC::mdl_v_expr(MDL_ADDRESS, $2.m_expr, true);
  }
  | tMINUS metric_expr %prec tNEG
  { 
    $$.m_expr = new T_dyninstRPC::mdl_v_expr(MDL_MINUS, $2.m_expr, true);
  }
  | metric_expr tDOT fields
  {
    $$.m_expr = new T_dyninstRPC::mdl_v_expr($1.m_expr, *$3.vs);
    delete $3.vs;
  } 
  | metric_expr bin_op metric_expr 
  {
    $$.m_expr = new T_dyninstRPC::mdl_v_expr($2.u, $1.m_expr, $3.m_expr);
  }
  | metric_expr u_op
  {
    $$.m_expr = new T_dyninstRPC::mdl_v_expr($2.u, $1.m_expr, false);
  }
  | tFUNCTION_CALL tLPAREN tLITERAL tCOMMA metric_expr_list tRPAREN 
  {
    $$.m_expr = new T_dyninstRPC::mdl_v_expr(*$3.sp, $5.m_expr_v); 
    delete $3.sp;
  } 
  | tIDENT tLPAREN metric_expr_list tRPAREN
  {
    $$.m_expr = new T_dyninstRPC::mdl_v_expr(*$1.sp, $3.m_expr_v); 
    delete $1.sp;
  } 
  | tARG tLSQUARE metric_expr tRSQUARE 
  {
    $$.m_expr = new T_dyninstRPC::mdl_v_expr("$arg", $3.m_expr);
  }
  | tIDENT tLSQUARE metric_expr tRSQUARE 
  {
    $$.m_expr = new T_dyninstRPC::mdl_v_expr(*$1.sp, $3.m_expr);
    delete $1.sp;
  }
  | tLPAREN metric_expr tRPAREN { $$.m_expr = $2.m_expr; }
  ;

where_instr: tPRE_INSN   { $$.u = MDL_PRE_INSN; }
  | tPOST_INSN  { $$.u = MDL_POST_INSN; }
  ;

instr_request: position where_instr metric_expr opt_constrained tLC instr_code_list tRC 
  {
    $$.m_stmt = new T_dyninstRPC::mdl_instr_stmt(
      $1.u, $3.m_expr, $6.icode_v, $2.u, $4.b);
  }
  ;

metric_stmts: { $$.m_stmt_v = new vector<T_dyninstRPC::mdl_stmt*>;}
  | metric_stmts metric_stmt 
  { $$.m_stmt_v = $1.m_stmt_v; (*$$.m_stmt_v) += $2.m_stmt;}
  ;

metric_stmt: tLBLOCK metric_stmts tRBLOCK 
    { $$.m_stmt = new T_dyninstRPC::mdl_seq_stmt($2.m_stmt_v); }
  | tFOREACH tIDENT tIN metric_expr metric_stmt 
  { 
    $$.m_stmt = new T_dyninstRPC::mdl_for_stmt(*$2.sp, $4.m_expr, $5.m_stmt);
    delete $2.sp; 
  }
  | tIF tLPAREN metric_expr tRPAREN metric_stmt 
    { $$.m_stmt = new T_dyninstRPC::mdl_if_stmt($3.m_expr, $5.m_stmt); }
  | instr_request { $$.m_stmt = $1.m_stmt; }
  ;

flavor_list: tIDENT 
		{ $$.vs = new vector<string>; (*$$.vs) += *$1.sp; delete $1.sp; }
	| flavor_list tCOMMA tIDENT 
		{ $$.vs = $1.vs; (*$$.vs) += *$3.sp; delete $3.sp; }
	;

met_flavor: tFLAVOR tASSIGN tLBLOCK flavor_list tRBLOCK tSEMI {$$.vs = $4.vs;}
	| tFLAVOR tLBLOCK flavor_list tRBLOCK tSEMI { $$.vs = $3.vs;}
	;

mode_val: tDEVELOPER { $$.b = true; }
	| tNORMAL { $$.b = false; }
	; 

met_mode: tMODE mode_val tSEMI { $$.b = $2.b; }
	;

unittype_val: tNORMALIZE { $$.i = 1; }
	| tUNNORMALIZE { $$.i = 0; }
	| tSAMPLED { $$.i = 2; }
	;

met_unittype: tUNITTYPE unittype_val tSEMI {$$.i = $2.i;}
	;	

met_base: tBASE tIS tCOUNTER tLBLOCK metric_stmts tRBLOCK 
	{
		$$.base.type = MDL_T_COUNTER;
		$$.base.m_stmt_v = $5.m_stmt_v;
	}
	| tBASE tIS tP_TIME tLBLOCK metric_stmts tRBLOCK 
	{
		$$.base.type = MDL_T_PROC_TIMER;
		$$.base.m_stmt_v = $5.m_stmt_v; 
	}
	| tBASE tIS tW_TIME tLBLOCK metric_stmts tRBLOCK 
	{
		$$.base.type = MDL_T_WALL_TIMER;
		$$.base.m_stmt_v = $5.m_stmt_v; 
	}
	| tBASE tIS tVOID tLBLOCK metric_stmts tRBLOCK 
	{
		// for special un-sampled metrics.
		$$.base.type = MDL_T_NONE;
		$$.base.m_stmt_v = $5.m_stmt_v; 
	}
	;

constraint_list: tCONSTRAINT tIDENT tSEMI 
	{
		T_dyninstRPC::mdl_constraint *c =
			mdl_data::new_constraint(*$2.sp, NULL, NULL, false, MDL_T_COUNTER);
		delete $2.sp;
		$$.mfld = new metricFld;
		$$.mfld->constraint = c; 
	}
	| int_constraint_definition 
	{
		$$.mfld = new metricFld;
	    $$.mfld->constraint = $1.constraint; 
	}
	;

met_temps: tCOUNTER tIDENT tSEMI { $$.sp = $2.sp; }
	;

metric_struct: tLBLOCK metric_list tRBLOCK {$$.mde = $2.mde;}
  ;

metric_list: {$$.mde = new metricDef();}
  | metric_list metric_elem_list 
  { $$.mde = $1.mde; $$.mde->setField(*$2.mfld); delete $2.mfld }
  ;

metric_elem_list: met_name 
  {
    $$.mfld = new metricFld;
    $$.mfld->name = *$1.sp; $$.mfld->spec = SET_MNAME;
    delete $1.sp;
  }
  | met_units 
  {
    $$.mfld = new metricFld;
    $$.mfld->units = *$1.sp; $$.mfld->spec = SET_UNITS;
    delete $1.sp;
  }
  | met_agg 
  {
    $$.mfld = new metricFld;
    $$.mfld->agg = $1.u; $$.mfld->spec = SET_AGG;
  }
  | met_style 
  {
    $$.mfld = new metricFld;
    $$.mfld->style = $1.u; $$.mfld->spec = SET_STYLE;
  }
  | met_flavor 
  {
    $$.mfld = new metricFld;
    $$.mfld->flavor = $1.vs;
    $$.mfld->spec = SET_MFLAVOR;
  }
  | met_mode 
  {
    $$.mfld = new metricFld;
    $$.mfld->mode = $1.b; $$.mfld->spec = SET_MODE;
  }
  | constraint_list 
  {
    $$.mfld = new metricFld;
    $$.mfld->constraint = $1.mfld->constraint;
    $$.mfld->spec = SET_CONSTRAINT;
    delete $1.mfld;
  }
  | met_temps 
  {
    $$.mfld = new metricFld;
    $$.mfld->temps = *$1.sp;
    $$.mfld->spec = SET_TEMPS;
  }
  | met_base 
  {
    $$.mfld = new metricFld;
    $$.mfld->base_type = $1.base.type;
    $$.mfld->base_m_stmt_v = $1.base.m_stmt_v;
    $$.mfld->spec = SET_BASE;
  }
  | met_unittype
  { 
    $$.mfld = new metricFld;
    $$.mfld->unittype = $1.i; 
    $$.mfld->spec = SET_UNITTYPE;
  }
  ;

metric_definition: tMETRIC tIDENT metric_struct 
  {
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
  }
  ;

match_path: {$$.vs = new vector<string>; }
  | match_path tDIV tIDENT 
    { $$.vs = $1.vs; (*$$.vs) += *$3.sp; delete $3.sp; }
  | match_path tDIV tMULT 
    { $$.vs = $1.vs; (*$$.vs) += "*"; }
  ;

def_constraint_definition: tCONSTRAINT tIDENT match_path tIS tDEFAULT tSEMI 
  {
    T_dyninstRPC::mdl_constraint *c = mdl_data::new_constraint(
      *$2.sp, $3.vs, NULL, false, MDL_T_NONE);
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
  ;

ext_constraint_definition: tCONSTRAINT tIDENT match_path tIS tCOUNTER tLBLOCK metric_stmts tRBLOCK 
  {
    T_dyninstRPC::mdl_constraint *c = mdl_data::new_constraint(
      *$2.sp, $3.vs, $7.m_stmt_v, false, MDL_T_COUNTER);
    if (!c) {
      yyerror("Error, did not new mdl_constraint\n");
      exit(-1);
    } else {
      mdl_data::unique_name(*$2.sp);
      mdl_data::all_constraints += c;
      delete $2.sp; 
    }
  }
  ;

cons_type: tCOUNTER  { $$.u = MDL_T_COUNTER; }
  | tW_TIME    { $$.u = MDL_T_WALL_TIMER; }
  | tP_TIME    { $$.u = MDL_T_PROC_TIMER; }
  ;

int_constraint_definition: tCONSTRAINT tIDENT match_path tIS tREPLACE cons_type tLBLOCK metric_stmts tRBLOCK 
  {
    $$.constraint = mdl_data::new_constraint(
      *$2.sp, $3.vs, $8.m_stmt_v, true, $6.u);
    if (!$$.constraint) {
      string msg = string("Error, did not new mdl_constraint\n");
      yyerror(P_strdup(msg.string_of()));
      exit(-1);
    }
    delete $2.sp; 
  }
  ;


