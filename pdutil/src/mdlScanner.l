%{
/*
 * Copyright (c) 1996-2003 Barton P. Miller
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
 * $Id: mdlScanner.l,v 1.3 2003/07/15 22:47:38 schendel Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define YYSTYPE struct parseStack
#include "y.tab.h"
#include "pdutil/h/mdlParse.h"

// Seems that these are not used anywhere. Comment them out. --chun
//int cur_line = 1;
//char cur_file[80];
//int yylook();
//int yyback(int*,int);

extern void mdlerror(const char *);

int curr;
int len;
int lineNo = 0;
#define MDL_MAX 255
char line[MDL_MAX];

// for use when parsing MDL out of a buffer in memory
// instead of from reading a file
const char* mdlBufPtr = NULL;
unsigned int mdlBufRemaining = 0;

#undef getc
inline
int
getScanChar(FILE *file)
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

inline
int
mdlGetInput( char* buf, unsigned int max_size )
{
    int ret;

    if( mdlBufPtr != NULL )
    {
        // we're parsing MDL from a buffer in memory
        // we assume mdlBufPtr and mdlBufRemaining are valid
        unsigned int nConsumed = 
            (( mdlBufRemaining < max_size ) ? mdlBufRemaining : max_size);
        memcpy( buf, mdlBufPtr, nConsumed );
        mdlBufPtr += nConsumed;
        mdlBufRemaining -= nConsumed;
        ret = nConsumed;
    }
    else
    {
        assert( mdlin != NULL );

        // we're reading from a file
        int c = getScanChar(mdlin);
        ret = (c == EOF) ? YY_NULL : (buf[0] = c, 1);
    }
    return ret;
}

#ifdef FLEX_SCANNER
#undef YY_INPUT
#define YY_INPUT(buf,result,max_size)   \
{                                       \
    result = mdlGetInput( buf, max_size ); \
}
#endif

%}
DIGIT	[0-9]
DOLLAR	\$
INT	{DIGIT}+
FIX	{INT}"."{INT}
ALPHA	[A-Za-z]
SLASH	\/
IDENT	{DOLLAR}?(_|{ALPHA})({ALPHA}|{DIGIT}|_)*
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
"metfocus"	{ return(tMETFOCUS); }
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
"hwTimer"       {return(tHW_TIME);}
"hwCounter"       {return(tHW_COUNTER);}
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
"noCase"		{ return(tNOCASE); }
"regex"		{ return(tREGEX); }
"tunable_constant"	{ return(tTUNABLE_CONSTANT); }
"command"		{ return(tCOMMAND); }
"remoteShell"          { return(tREMOTE_SHELL); }
"host"		{ return(tHOST); }
"dir"		{ return(tDIR); }
"user"		{ return(tUSER);}
"autoStart"     { return(tAUTO_START); }
"force"		{ return(tFORCE);}
"limit"		{ return(tLIMIT);}
"metric"	{ return(tMETRIC);}
"default"	{ return(tDEFAULT); }
"void"		{ return(tVOID); }
"items"         { return(tITEMS); }
"library"       { return(tLIBRARY); }
"setCounter"    { mdlerror("\'setCounter\' is obsolete. Please make sure "
                    "that you are using a up-to-date paradyn.rc.");
                  exit(1); 
                }
"subCounter"    { mdlerror("\'subCounter\' is obsolete. Please make sure "
                    "that you are using a up-to-date paradyn.rc.");
                  exit(1); 
                }
"addCounter"    { mdlerror("\'addCounter\' is obsolete. Please make sure "
                    "that you are using a up-to-date paradyn.rc.");
                  exit(1); 
                }
"functionCall"  { mdlerror("\'functionCall\' is obsolete.  It's replaced "
                    "by \'Call\'");
                  exit(1); 
                }
[Tt][Rr][Uu][Ee]	{ return(tTRUE);}
[Ff][Aa][Ll][Ss][Ee]	{ return(tFALSE);}
{FIX}		{ 
		  mdllval.f = atof(yytext);
		  return(tFLOAT);
		}
{COMMENT}	BEGIN(mdlcomment);
<mdlcomment>[^\n]*\n	{ BEGIN(INITIAL); }
{IDENT}		{
            mdllval.sp = new pdstring(yytext);
            return(tIDENT);
            }
{INT}           {
		  int i = atoi(yytext);
                  assert(i >= 0);
                  mdllval.u = (unsigned) i;
		  return(tUNS);
		}
[ \n\t]+		{ }
\"              BEGIN(mdlstr);
<mdlstr>\n	{ mdlerror("Unterminated string literal"); }
<mdlstr>\"	{ /* we are done with the string */
			BEGIN(INITIAL); return (tLITERAL); 
			}
<mdlstr>[^\"\n]*	{mdllval.sp = new pdstring(yytext); }
.       	{ mdlerror("invalid character"); }
%%

void mdlerror(const char *message)
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

int mdlwrap( void )
{
    return 1;
}
