%{
/*
 * $Log: metScanner.l,v $
 * Revision 1.3  1995/02/07 21:59:56  newhall
 * added a force option to the visualization definition, this specifies
 * if the visi should be started before metric/focus menuing
 * removed compiler warnings
 *
 * Revision 1.2  1994/08/22  15:53:30  markc
 * Config language version 2.
 *
 * Revision 1.1  1994/07/07  03:25:28  markc
 * Configuration language parser.
 *
 *
 */
#ifndef lint
static char Copyright[] =
    "@(#) Copyright (c) 1992 Jeff Hollingsworth. All rights reserved.";

static char rcsid[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/paradyn/src/met/Attic/metScanner.l,v 1.3 1995/02/07 21:59:56 newhall Exp $";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define YYSTYPE struct parseStack
#include "y.tab.h"
#include "paradyn/src/met/metParse.h"

int cur_line = 1;
char cur_file[80];

int yylook();
int yyback(int*,int);


extern void yyerror(char *);

int curr;
int len;
int lineNo = 0;
#define MAX 255
char line[MAX];

#undef getc
int getScanChar(FILE *file)
{
    char *ret;

    if (curr == len) {
	ret = fgets(line, MAX, file);
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
IDENT	{DOLLAR}?{ALPHA}({ALPHA}|{DIGIT}|_|".")*
COMMENT	{SLASH}{SLASH}
%%
"}"		{ return(tRBRAC); }
"{"		{ return(tLBRAC); }
","		{ return(tCOMMA); }
";"		{ return(tSEMI); }
daemon		{ return(tDAEMON); }
process		{ return(tPROCESS); }
visi		{ return(tVISI); }
tunable_constant	{ return(tTUNABLE_CONSTANT); }
command		{ return(tCOMMAND); }
host		{ return(tHOST); }
flavor		{ return(tFLAVOR); }
pvm		{ return(tPVM); }
cm5		{ return(tCM5); }
unix		{ return(tUNIX); }
name		{ return(tNAME); }
list		{ return(tLIST); }
dir		{ return(tDIR); }
user		{ return(tUSER);}
force		{ return(tFORCE);}
[Tt][Rr][Uu][Ee]	{ return(tTRUE);}
[Ff][Aa][Ll][Ss][Ee]	{ return(tFALSE);}
{INT}		{
		  yylval.i = atoi(yytext);
		  return(tINT);
		}
{FIX}		{ 
		  yylval.f = atof(yytext);
		  return(tFLOAT);
		}
{COMMENT}	{
		    char ch;
		    while (1) {
			ch = getScanChar(yyin);
			if (ch == '\n') break;
		    }
		}
{IDENT}		{ /* it's a identifier */
			  char *temp;

			  if (*yytext == '$') yytext++;
			  temp = (char *) malloc(strlen(yytext) + 1);
			  strcpy(temp, yytext);
			  yylval.cp = temp;
			  return(tIDENT);
		}
\"              {
			  extern void doString();

			  yyleng = 0;
			  doString();
			  yylval.cp = strcpy((char*) malloc(yyleng+1), yytext);
			  return(tLITERAL);
		}
[ \t\n]	{ /* ignore white space */ }
.	{ yyerror("invalid character"); }
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
    printf("line %d, col %d:%s\n", lineNo, curr, message);
    printf(line);
    for (i=0; i < curr; i++) {
	printf(" ");
    }
    printf("^\n");
}
