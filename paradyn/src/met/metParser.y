
%{ 

/*
 * $Log: metParser.y,v $
 * Revision 1.5  1995/05/18 10:58:33  markc
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
extern void *yyerror(char *);
extern int yyparse();
extern void handle_error();
extern bool hack_in_cons;
extern void hack_cons_type(vector<string>*);
extern unsigned hacked_cons_type;
%}

%token tDAEMON tPROCESS tTUNABLE_CONSTANT tIDENT tINT
%token tCOMMAND tARGS tHOST tLITERAL tFLOAT tCOMMA
%token tSEMI tFLAVOR tNAME
%token tRES_LIST tVISI tUSER tDIR tFALSE tTRUE tFORCE

%token tT_PROCEDURE tT_MODULE tT_STRING tT_INT tT_FLOAT tTRUE tFALSE tDEFAULT
%token tFOREACH tLPAREN tRPAREN tLBLOCK tRBLOCK tCOLON tDOLLAR
%token tMETRIC tUNS tBASE tNAME tUNITS tFOLD tBASE tIS tFUNCTION_CALL
%token tCOUNTER tAGG tAVG tSUM tMIN tMAX tDOT tW_TIME tP_TIME
%token tAPPEND tPREPEND tDERIVED tIF tREPLACE tCONSTRAINT tCONSTRAINED
%token tTYPE tAT tIN tLSQUARE tRSQUARE tBEFORE tAFTER
%token tSTYLE tEVENT_COUNTER tSAMPLE_FUNC
%token tPLUS tMINUS tDIV tMULT tLT tGT tLE tGE tEQ tNE tAND tOR tNOT
%token tADD_COUNTER tSET_COUNTER tSUB_COUNTER 
%token tSTART_PROC_TIMER tSTOP_PROC_TIMER 
%token tSTART_WALL_TIMER tSTOP_WALL_TIMER 
%token tRC tLC tASSIGN tPRE_INSN tPOST_INSN
%token tARG tRETURN
%token tREAD_SYMBOL tREAD_ADDRESS
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
	  | error
          | metric_definition
          | ext_constraint_definition 
          | def_constraint_definition
          | metric_stmt {    mdl_data::stmts += $1.m_stmt; };

list_type: tT_STRING       { $$.u = MDL_T_STRING; }
         | tT_PROCEDURE    { $$.u = MDL_T_PROCEDURE; }
         | tT_MODULE       { $$.u = MDL_T_MODULE; }
         | tT_FLOAT        { $$.u = MDL_T_FLOAT; }
         | tT_INT          { $$.u = MDL_T_INT; };

opt_library: tCOMMA tTRUE     { $$.b = true; }
           | tCOMMA tFALSE    { $$.b = false; };

list_id: tCOMMA tLITERAL      { $$.sp = $2.sp; }

list_items: tCOMMA tLPAREN stringItems tRPAREN { $$.vs = $3.vs; }

resList: tRES_LIST tLPAREN list_type list_id list_items opt_library tCOMMA tIDENT tRPAREN tSEMI { 
  metParseError = ERR_NO_ERROR;
  T_dyninstRPC::mdl_stmt *s = new T_dyninstRPC::mdl_list_stmt($3.u, *$4.sp,
							      $5.vs, $6.b, *$8.sp);
  if (s) mdl_data::stmts += s;
  delete $4.sp; delete $8.sp;
} | tRES_LIST error;

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
     | tHOST tIDENT tSEMI
           { $$.fld.val = $2.sp; $$.fld.spec = SET_HOST;}
     | tUSER tIDENT tSEMI
           { $$.fld.val = $2.sp; $$.fld.spec = SET_USER;}
     | tDIR tIDENT tSEMI
           { $$.fld.val = $2.sp; $$.fld.spec = SET_DIR;};
     | tFORCE tINT tSEMI
           { $$.fld.force = $2.i; $$.fld.spec = SET_FORCE;};

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
     | tHOST tIDENT tSEMI
           { $$.fld.val = $2.sp; $$.fld.spec = SET_HOST;}
     | tUSER tIDENT tSEMI
           { $$.fld.val = $2.sp; $$.fld.spec = SET_USER;}
     | tDIR tIDENT tSEMI
           { $$.fld.val = $2.sp; $$.fld.spec = SET_DIR;};

tunableConstant: tTUNABLE_CONSTANT tunableItem
               | tTUNABLE_CONSTANT tLBLOCK tunableList tRBLOCK
	       | tTUNABLE_CONSTANT tLBLOCK error tRBLOCK
	       | tTUNABLE_CONSTANT tLBLOCK error
	       | tTUNABLE_CONSTANT error;

tunableList: 
           | tunableList tunableItem;

tunableItem: tLITERAL tINT tSEMI  {
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
           | tHOST tIDENT tSEMI
               { $$.fld.val = $2.sp; $$.fld.spec = SET_HOST;}
           | tDIR tIDENT tSEMI
              { $$.fld.val = $2.sp; $$.fld.spec = SET_DIR;};

met_name: tNAME tLITERAL tSEMI { $$.sp = $2.sp; };

met_units: tUNITS tIDENT tSEMI { $$.sp = $2.sp; };

fold_val: tAVG { $$.u = MDL_FOLD_AVG;}
       | tSUM { $$.u = MDL_FOLD_SUM;};

met_fold: tFOLD fold_val tSEMI { $$.u = $2.u;};

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
  $$.expr.rand2.str = NULL; $$.expr.rand2.str2 = NULL;
} | instr_rand instr_op instr_rand {
  $$.expr.rand1 = $1.rand; $$.expr.rand2 = $3.rand; $$.expr.bin_op = $2.u;
};

instr_op: tLT       { $$.u = MDL_LT; }
        | tGT       { $$.u = MDL_GT; }
        | tLE       { $$.u = MDL_LE; }
        | tGE       { $$.u = MDL_GE; }
        | tEQ       { $$.u = MDL_EQ; }
        | tNE       { $$.u = MDL_NE; };

// TODO -- note the type for constraints is not known yet
// It could be known, but I am delaying the processing until apply is called
instr_rand: tIDENT                       {
                                           if (*$1.sp == "$constraint") {
					     char msg[100];
					     if (!hack_in_cons) {
					       sprintf(msg, "Error: using $constraint as a variable outside of constraint definition\n");
					       yyerror(msg);
					       exit(-1);
					     } else if (hacked_cons_type != MDL_T_INT) {
					       sprintf(msg, "Error: using $constraint with non-integer type in instrumentation expression\n");
					       yyerror(msg);
					       exit(-1);
					     }
					     $$.rand.type = MDL_T_INT;
					     $$.rand.str = new string("$constraint");
					     $$.rand.str2 = new string("s");
					   } else {
					     $$.rand.type = MDL_T_COUNTER;
					     $$.rand.str = $1.sp;
					     $$.rand.str2 = new string("s");
					   }
					 }
          | tUNS                         { $$.rand.type = MDL_T_INT; $$.rand.arg = $1.u;
					   $$.rand.str = new string("s");
					   $$.rand.str2 = new string("s"); }
          | tARG tLSQUARE tUNS tRSQUARE  { $$.rand.type = MDL_ARG; $$.rand.arg = $3.u;
					   $$.rand.str = new string("s");
					   $$.rand.str2 = new string("s"); }
          | tRETURN                      { $$.rand.type = MDL_RETURN; $$.rand.arg = 0;
					   $$.rand.str = new string("s"); 
					   $$.rand.str2 = new string("s"); }
          | tREAD_SYMBOL tLPAREN tLITERAL tRPAREN {
	                                   $$.rand.type = MDL_READ_SYMBOL; $$.rand.arg = 0;
					   $$.rand.str = $3.sp;
					   $$.rand.str2 = new string("s"); }
          | tREAD_ADDRESS tLPAREN tUNS tRPAREN {
                                           $$.rand.type = MDL_READ_ADDRESS;
					   $$.rand.arg = $3.u;
					   $$.rand.str = new string("s");
					   $$.rand.str2 = new string("s"); }
          | tFUNCTION_CALL tLPAREN tLITERAL tCOMMA tUNS tRPAREN  {
	                                   $$.rand.type = MDL_CALL_FUNC;
					   $$.rand.str = $3.sp; $$.rand.arg = $5.u;
					   $$.rand.str2 = new string("s"); }
          | tFUNCTION_CALL tLPAREN tLITERAL tCOMMA tIDENT tRPAREN  {
	                                   $$.rand.type = MDL_CALL_FUNC;
					   $$.rand.str = $3.sp; $$.rand.arg = $5.u;
					   $$.rand.str2 = $5.sp;
					 };

// TODO -- allow the function call to be passed $arg

instr_req: tSET_COUNTER tLPAREN tIDENT tCOMMA instr_rand tRPAREN tSEMI {
  $$.instr_req = new T_dyninstRPC::mdl_instr_req($5.rand.type, $5.rand.arg, *$5.rand.str,
						 *$5.rand.str2, MDL_SET_COUNTER, *$3.sp);
  delete $3.sp; delete $5.rand.str; delete $5.rand.str2;
} | tADD_COUNTER tLPAREN tIDENT tCOMMA instr_rand tRPAREN tSEMI {
  $$.instr_req = new T_dyninstRPC::mdl_instr_req($5.rand.type, $5.rand.arg, *$5.rand.str,
						 *$5.rand.str2, MDL_ADD_COUNTER, *$3.sp);
  delete $3.sp; delete $5.rand.str; delete $5.rand.str2;
} | tSUB_COUNTER tLPAREN tIDENT tCOMMA instr_rand tRPAREN tSEMI {
  $$.instr_req = new T_dyninstRPC::mdl_instr_req($5.rand.type, $5.rand.arg, *$5.rand.str,
						 *$5.rand.str2, MDL_SUB_COUNTER, *$3.sp);
  delete $3.sp; delete $5.rand.str; delete $5.rand.str2;
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
} | tIDENT tLPAREN instr_rand tRPAREN {
  $$.instr_req = new T_dyninstRPC::mdl_instr_req($3.rand.type, $3.rand.arg, *$3.rand.str,
						 *$3.rand.str2, MDL_CALL_FUNC, *$1.sp);
  delete $1.sp; delete $3.rand.str; delete $3.rand.str2;
};

instr_code: tIF tLPAREN instr_expr tRPAREN instr_req {
  $$.i_code = new T_dyninstRPC::mdl_icode($3.expr.rand1.type, $3.expr.rand1.arg,
					  *$3.expr.rand1.str,
					  $3.expr.rand2.type, $3.expr.rand2.arg,
					  *$3.expr.rand2.str,
					  $3.expr.bin_op, true, $5.instr_req);
  delete $3.expr.rand1.str; delete $3.expr.rand2.str; 
  delete $3.expr.rand1.str2; delete $3.expr.rand2.str2; 
} | instr_req {
  string empty;
  $$.i_code = new T_dyninstRPC::mdl_icode(0, 0, empty,
					  0, 0, empty,
					  0, false, $1.instr_req);
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

variable: tIDENT           { $$.sp = $1.sp; }
        | tDOLLAR tIDENT   { string *s = new string(string("$") + *$2.sp);
			     $$.sp = s; delete $2.sp; }
        | tDOLLAR tCONSTRAINT { string *s = new string(string("$") + "constraint"); $$.sp = s;
			      };

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

met_flavor: tFLAVOR tASSIGN tLBLOCK flavor_list tRBLOCK tSEMI { $$.vs = $4.vs; };

met_base: tBASE tIS tCOUNTER tLBLOCK metric_stmts tRBLOCK {
  $$.base.type = MDL_T_COUNTER;
  $$.base.m_stmt_v = $5.m_stmt_v;
} | tBASE tIS tP_TIME tLBLOCK metric_stmts tRBLOCK { 
  $$.base.type = MDL_T_PROC_TIMER;
  $$.base.m_stmt_v = $5.m_stmt_v; 
} | tBASE tIS tW_TIME tLBLOCK metric_stmts tRBLOCK { 
  $$.base.type = MDL_T_WALL_TIMER;
  $$.base.m_stmt_v = $5.m_stmt_v; 
};

constraint_list:      {
  $$.v_cons = new vector<T_dyninstRPC::mdl_constraint*>;

} | constraint_list tCONSTRAINT tIDENT tSEMI {
  $$.v_cons = $1.v_cons;
 T_dyninstRPC::mdl_constraint *c =  mdl_data::new_constraint(*$3.sp,
							     NULL, NULL, false,
							     MDL_T_COUNTER);
  delete $3.sp;
  (*$$.v_cons) += c;

} | constraint_list int_constraint_definition {
  $$.v_cons = $1.v_cons;
  (*$$.v_cons) += $2.constraint;
};

met_temps:                                 { $$.vs = new vector<string>; }
         | met_temps tCOUNTER tIDENT tSEMI { $$.vs = $1.vs; (*$$.vs) += *$3.sp; delete $3.sp;};

metric_definition: tMETRIC tIDENT tLBLOCK met_name met_units met_fold met_agg met_style met_flavor constraint_list met_temps met_base tRBLOCK {
  if (!mdl_data::new_metric(*$2.sp, *$4.sp, *$5.sp, $6.u, $7.u, $8.u, $12.base.type, 
			    $12.base.m_stmt_v, $9.vs, $10.v_cons, $11.vs)) {
    char msg[100];
    sprintf(msg, "Error defining %s\n", $2.sp->string_of());
    yyerror(msg);
    exit(-1);
  }
  delete $2.sp;
  delete $4.sp;
  delete $5.sp;
};

match_path:                        {$$.vs = new vector<string>; }
          | match_path tDIV tIDENT { 
	                $$.vs = $1.vs; (*$$.vs) += *$3.sp; delete $3.sp;
			hack_cons_type($$.vs); 
		      };

def_constraint_definition: tCONSTRAINT tIDENT match_path tIS tDEFAULT tSEMI {
  T_dyninstRPC::mdl_constraint *c = mdl_data::new_constraint(*$2.sp, $3.vs, NULL, false,
							     MDL_T_NONE);
  if (!c) {
    char msg[100];
    sprintf(msg, "Error, did not new mdl_constraint\n");
    yyerror(msg);
    exit(-1);
  } else
    mdl_data::all_constraints += c;
  delete $2.sp; 
}

ext_constraint_definition: tCONSTRAINT tIDENT match_path tIS tCOUNTER tLBLOCK metric_stmts tRBLOCK {
  hack_in_cons = false;
  T_dyninstRPC::mdl_constraint *c = mdl_data::new_constraint(*$2.sp, $3.vs,
							     $7.m_stmt_v, false,
							     MDL_T_COUNTER);
    delete $2.sp; 

  if (!c) {
    char msg[100];
    sprintf(msg, "Error, did not new mdl_constraint\n");
    yyerror(msg);
    exit(-1);
  } else
    mdl_data::all_constraints += c;
};

cons_type: tCOUNTER  { $$.u = MDL_T_COUNTER; }
         | tW_TIME    { $$.u = MDL_T_WALL_TIMER; }
         | tP_TIME    { $$.u = MDL_T_PROC_TIMER; };

int_constraint_definition: tCONSTRAINT tIDENT match_path tIS tREPLACE cons_type tLBLOCK metric_stmts tRBLOCK {
  $$.constraint = mdl_data::new_constraint(*$2.sp, $3.vs, $8.m_stmt_v, true, $6.u);
  if (!$$.constraint) {
    char msg[100];
    sprintf(msg, "Error, did not new mdl_constraint\n");
    yyerror(msg);
    exit(-1);
  }
  delete $2.sp; 
};


