%{
/*
 * Copyright (c) 1996 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
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
 * $Log $
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define YYSTYPE union parse_stack
#include "y.tab.h"
#include "parse.h"

int parsing;
int cur_line = 1;
char cur_file[80];

int yylook();
int yyback(int*,int);

#ifdef notdef
#ifdef FLEX_SCANNER
/* flex defines this as a macro (bad flex!). */
#undef yywrap;
extern int yywrap();
#endif
#endif

extern void yyerror(char *);

int curr;
int len;
int lineNo = 0;
#define MAX_LINE 255
char line[MAX_LINE];
extern char *serverTypeName;

#undef getc
int getScanChar(FILE *file)
{
    char *ret;

    if (curr == len) {
	ret = fgets(line, MAX_LINE, file);
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
IDENT	{ALPHA}({ALPHA}|{DIGIT}|_|".")*
FWD     "forward"
%%
\$remote	{ parsing = 1; 	}
"//".*\n		{ if (!parsing) REJECT; }
.*\n		{
                  if (!parsing) {
		    if (!strncmp(yytext, "$remote",
				 strlen("$remote"))) {
		      REJECT;
		    } else {
		      dump_to_dot_h(yytext) ;
		    }
                  } else
		    REJECT;
		}
"forward"     	{ printf("forward!\n"); return(tFORWARD); }
\$nobundle      { return(tNOBUNDLE); }
\$upcall	{ return(tUPCALL); }
\$virtual	{ return(tVIRTUAL); }
\$base		{ return(tBASE); }
\$name		{ return(tNAME); }
\$version	{ return(tVERSION); }
\$async		{ return(tASYNC); }
\$free		{ return(tFREE); }
"&"		{ return(tREF); }
"("		{ return(tLPAREN); }
")"		{ return(tRPAREN); }
"{"		{ return(tLBLOCK); }
"}"		{ return(tRBLOCK); }
","		{ return(tCOMMA); }
"<"		{ return(tANGLE_L);}
">"		{ return(tANGLE_R);}
"*"		{ return(tSTAR); }
":"		{ return(tCOLON); }
";"		{ /* parsing = 0; */
		  return(tSEMI); 
		}
"const"         { return(tCONST); }
"typedef"	{ return(tTYPEDEF); }
"struct"	{ return(tSTRUCT); }
"class"		{ return(tCLASS); }
"abstract"	{ return(tABSTRACT); }
\$cignore[^$]*\$cignore		{ yylval.charp = yytext; return(tCIGNORE); }
\$signore[^$]*\$signore		{ yylval.charp = yytext; return(tSIGNORE); }
\$ignore[^$]*\$ignore		{ yylval.charp = yytext; return(tIGNORE); }
{IDENT}		{ /* it's an identifier */
                  yylval.cp = new pdstring(yytext);
		  return(tIDENT);
		}
[ \n\t]		{ }
{INT}           {
		  int i = atoi(yytext);
                  assert(i >= 0);
                  yylval.u = (unsigned) i;
		  return(tUNS);
		}
.       	{ yyerror("invalid character"); }
%%

void doString()
{
    char ch;

    while (1) {
	ch = getScanChar(yyin);
	switch (ch) {
	    case '"':
		yytext[yyleng] = '\0';
		return;
		break;
	    
	    case '\n':
		yyerror("Unterminated string literal");
		return;
		break;

	    default:
		yytext[yyleng] = ch;
		break;
	}
	yyleng++;
    }
}

void yyerror(char *message)
{
    int i;
    fprintf(stderr, "line %d, col %d:%s\n", lineNo, curr, message);
    fprintf(stderr, line);
    for (i=0; i < curr; i++) {
	fprintf(stderr, " ");
    }
    fprintf(stderr, "^\n");
}
