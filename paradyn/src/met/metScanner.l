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
 * $Log: metScanner.l,v $
 * Revision 1.19  1998/01/28 16:31:50  czhang
 * Check for obsolete words "setCounter", "subCounter", "addCounter",
 * "functionCall".  If there's any, bail out and warn user.  Fixed a bug about
 * the inaccuracy of the line number reporting.
 *
 * Revision 1.18  1998/01/19 21:05:53  czhang
 * Massive changes for MDL expression.
 * Changes to tokens are in metScanner.l.  This includes adding of new
 * operators.  Some obsolete keywords are removed.
 * Changes to the mdl expression are in metParser.y.  Rules associated with
 * the instrumentation expression (obsolete) are removed and the mdl expression
 * is expanded.
 * Other files, mdl.C, mdl.h, metParse.h contain changes due to the grammer,
 * this includes support for new mdl expression types, etc.
 *
 * Revision 1.17  1997/06/24 17:13:19  newhall
 * changed exclude_node to exclude
 *
 * Revision 1.16  1997/06/07 21:01:26  newhall
 * replaced exclude_func and exclude_lib with exclude_node
 *
 * Revision 1.15  1997/06/05 04:29:47  newhall
 * added exclude_func mdl option to exclude shared object functions
 *
 * Revision 1.14  1996/10/08 21:52:18  mjrg
 * changed the evaluation of resource lists
 * removed warnings
 *
 * Revision 1.13  1996/09/26 19:03:27  newhall
 * added "exclude_lib" mdl option
 *
 * Revision 1.12  1996/04/04  21:55:28  newhall
 * added limit option to visi definition
 *
 * Revision 1.11  1996/03/20  17:04:22  mjrg
 * Changed mdl to support calls with multiple arguments.
 *
 * Revision 1.10  1995/12/18 23:22:14  newhall
 * changed metric units type so that it can have one of 3 values (normalized,
 * unnormalized or sampled)
 *
 * Revision 1.9  1995/12/15 22:30:09  mjrg
 * Merged paradynd and paradyndPVM
 * Get module name for functions from symbol table in solaris
 * Fixed code generation for multiple instrumentation statements
 * Change syntax of MDL resource lists
 *
 * Revision 1.8  1995/11/21 15:15:39  naim
 * Changing the MDL grammar to allow more flexible metric definitions (i.e. we
 * can specify all elements in any order). Additionally, the option "fold"
 * has been removed - naim
 *
 * Revision 1.7  1995/11/17  17:22:16  newhall
 * added "unitsType" option to MDL, can be "normalized" or "unnormalized"
 *
 * Revision 1.6  1995/11/13  14:54:06  naim
 * Adding "mode" option to the Metric Description Language to allow specificacion
 * of developer mode for metrics (default mode is "normal") - naim
 *
 * Revision 1.5  1995/08/24  15:02:48  hollings
 * AIX/SP-2 port (including option for split instruction/data heaps)
 * Tracing of rexec (correctly spawns a paradynd if needed)
 * Added rtinst function to read getrusage stats (can now be used in metrics)
 * Critical Path
 * Improved Error reporting in MDL sematic checks
 * Fixed MDL Function call statement
 * Fixed bugs in TK usage (strings passed where UID expected)
 *
 * Revision 1.4  1995/05/18  10:58:35  markc
 * mdl
 *
 * Revision 1.3  1995/02/07  21:59:56  newhall
 * added a force option to the visualization definition, this specifies
 * if the visi should be started before metric/focus menuing
 * removed compiler warning * Revision 1.2  1994/08/22  15:53:30  markc
 * Config language version 2.
 *
 * Revision 1.1  1994/07/07  03:25:28  markc
 * Configuration language parser.
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define YYSTYPE struct parseStack
#include "y.tab.h"
#include "paradyn/src/met/metParse.h"

// Seems that these are not used anywhere. Comment them out. --chun
//int cur_line = 1;
//char cur_file[80];
//int yylook();
//int yyback(int*,int);

extern void yyerror(const char *);

int curr;
int len;
int lineNo = 0;
#define MDL_MAX 255
char line[MDL_MAX];

#undef getc
int getScanChar(FILE *file)
{
  char *ret;

  if (curr == len) {
    ret = fgets(line, MDL_MAX, file);
    if (!ret) return(EOF);
    lineNo++;
    len = strlen(line);
    curr = 0;
  }
  return(line[curr++]);
}

#ifdef FLEX_SCANNER
#undef YY_INPUT
#define YY_INPUT(buf,result,max_size)   \
{                                       \
    int c = getScanChar(yyin);		\
    result = (c == EOF) ? YY_NULL : (buf[0] = c, 1); \
}
#endif

%}
DIGIT	[0-9]
DOLLAR	\$
INT	{DIGIT}+
FIX	{INT}"."{INT}
ALPHA	[A-Za-z]
SLASH	\/
IDENT	{DOLLAR}?{ALPHA}({ALPHA}|{DIGIT}|_)*
COMMENT	{SLASH}{SLASH}
%x mdlcomment mdlstr

%%
"&"             { return(tAMPERSAND);}
"$"		{ return(tDOLLAR);}
"="		{ return(tASSIGN);}
"+"		{ return(tPLUS);}
"+="    { return(tPLUSASSIGN);}
"-="    { return(tMINUSASSIGN);}
"-"		{ return(tMINUS);}
"/"		{ return(tDIV);}
"*"		{ return(tMULT);}
"<"		{ return(tLT);}
">"		{ return(tGT);}
"<="		{ return(tLE);}
">="		{ return(tGE);}
"=="		{ return(tEQ);}
"!="		{ return(tNE);}
"&&"		{ return(tAND);}
"||"		{ return(tOR);}
"("		{ return(tLPAREN); }
")"		{ return(tRPAREN); }
"{"		{ return(tLBLOCK); }
"}"		{ return(tRBLOCK); }
"["		{ return(tLSQUARE);}
"]"		{ return(tRSQUARE);}
","		{ return(tCOMMA); }
";"		{ return(tSEMI); }
"."		{ return(tDOT); }
"++"    { return(tPLUSPLUS); }
"(*"		{ return(tLC); }
"*)"		{ return(tRC); }
"true"          { return(tTRUE); }
"false"         { return(tFALSE); }
"Call"	{ return(tFUNCTION_CALL); }
"resourceList"	{ return(tRES_LIST); }
"procedure"	{ return(tT_PROCEDURE); }
"module"	{ return(tT_MODULE); }
"string"	{ return(tT_STRING); }
"int"		{ return(tT_INT); }
"float"		{ return(tT_FLOAT); }
"flavor"	{ return(tFLAVOR);}
"foreach"	{ return(tFOREACH); }
"metric"	{ return(tMETRIC); }
"avg"		{ return(tAVG);}
"min"		{ return(tMIN);}
"max"		{ return(tMAX);}
"sum"		{ return(tSUM);}
"name"		{ return(tNAME);}
"units"		{ return(tUNITS);}
"mode"		{ return(tMODE);}
"developer"	{ return(tDEVELOPER);}
"normal"	{ return(tNORMAL);}
"unitsType"	{ return(tUNITTYPE);}
"normalized"	{ return(tNORMALIZE);}
"unnormalized"	{ return(tUNNORMALIZE);}
"sampled"	{ return(tSAMPLED);}
"base"		{ return(tBASE);}
"is"		{ return(tIS);}
"append"	{ return(tAPPEND);}
"prepend"	{ return(tPREPEND);}
"derived"	{ return(tDERIVED);}
"if"		{ return(tIF);}
"replace"	{ return(tREPLACE);}
"constraint"	{ return(tCONSTRAINT);}
"constrained"	{ return(tCONSTRAINED);}
"in"		{ return(tIN);}
"counter"	{ return(tCOUNTER);}
"processTimer"		{ return(tP_TIME);}
"wallTimer"		{ return(tW_TIME);}
"aggregateOperator" 	{ return(tAGG);}
"style"			{ return(tSTYLE);}
"EventCounter"		{ return(tEVENT_COUNTER); }
"SampledFunction"	{ return(tSAMPLE_FUNC); }
"preInsn"		{ return(tPRE_INSN); }
"postInsn"		{ return(tPOST_INSN); }
\$return		{ return(tRETURN);}
\$arg           { return(tARG);}
"daemon"		{ return(tDAEMON); }
"process"		{ return(tPROCESS); }
"visi"		{ return(tVISI); }
"exclude"		{ return(tEXLIB); }
"tunable_constant"	{ return(tTUNABLE_CONSTANT); }
"command"		{ return(tCOMMAND); }
"host"		{ return(tHOST); }
"dir"		{ return(tDIR); }
"user"		{ return(tUSER);}
"force"		{ return(tFORCE);}
"limit"		{ return(tLIMIT);}
"default"	{ return(tDEFAULT); }
"void"		{ return(tVOID); }
"items"         { return(tITEMS); }
"library"       { return(tLIBRARY); }
"setCounter"    { yyerror("\'setCounter\' is obsolete. Please make sure "
                    "that you are using a up-to-date paradyn.rc.");
                  exit(1); 
                }
"subCounter"    { yyerror("\'subCounter\' is obsolete. Please make sure "
                    "that you are using a up-to-date paradyn.rc.");
                  exit(1); 
                }
"addCounter"    { yyerror("\'addCounter\' is obsolete. Please make sure "
                    "that you are using a up-to-date paradyn.rc.");
                  exit(1); 
                }
"functionCall"  { yyerror("\'functionCall\' is obsolete.  It's replaced "
                    "by \'Call\'");
                  exit(1); 
                }
[Tt][Rr][Uu][Ee]	{ return(tTRUE);}
[Ff][Aa][Ll][Ss][Ee]	{ return(tFALSE);}
{FIX}		{ 
		  yylval.f = atof(yytext);
		  return(tFLOAT);
		}
{COMMENT}	BEGIN(mdlcomment);
<mdlcomment>[^\n]*\n	{ BEGIN(INITIAL); }
{IDENT}		{
            yylval.sp = new string(yytext);
            return(tIDENT);
            }
{INT}           {
		  int i = atoi(yytext);
                  assert(i >= 0);
                  yylval.u = (unsigned) i;
		  return(tUNS);
		}
[ \n\t]+		{ }
\"              BEGIN(mdlstr);
<mdlstr>\n	{ yyerror("Unterminated string literal"); }
<mdlstr>\"	{ /* we are done with the string */
			BEGIN(INITIAL); return (tLITERAL); 
			}
<mdlstr>[^\"\n]*	{yylval.sp = new string(yytext); }
.       	{ yyerror("invalid character"); }
%%

void yyerror(const char *message)
{
    int i;
    // curr is one pass the column of the current token --chun
    printf("line %d, col %d: %s\n", lineNo, curr-1, message);
    printf(line);
    for (i=0; i < curr-2; i++) {
        printf(" ");
    }
    printf("^\n");
}

