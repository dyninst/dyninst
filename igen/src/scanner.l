%{
/*
 * $Log $
 *
 */
#ifndef lint
static char Copyright[] =
    "@(#) Copyright (c) 1992 Jeff Hollingsworth. All rights reserved.";

static char rcsid[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/igen/src/scanner.l,v 1.4 1994/02/08 00:17:59 hollings Exp $";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define YYSTYPE union parseStack
#include "y.tab.h"
#include "../src/parse.h"
#include "util/h/stringPool.h"

int parsing;
int cur_line = 1;
char cur_file[80];
extern int emitHeader;
extern stringPool pool;

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
#define MAX 255
char line[MAX];
extern char *serverTypeName;

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
IDENT	{ALPHA}({ALPHA}|{DIGIT}|_|".")*
%%
\$remote	{ parsing = 1; 	}
"//".*\n		{ if (!parsing) REJECT; }
.*\n		{ if (!parsing) {
		    if (!strncmp(yytext, "$remote", strlen("$remote"))) {
			REJECT;
		    } else {
			if (emitHeader) ECHO; 
		    }
	          } else {
		    REJECT;
		  }
		}
\$upcall	{ return(tUPCALL); }
\$base		{ return(tBASE); }
\$name		{ return(tNAME); }
\$version	{ return(tVERSION); }
\$async		{ return(tASYNC); }
\$array		{ return(tARRAY); }
\$server	{ yylval.cp = serverTypeName;
		  return(tIDENT);
		}
"("		{ return(tLPAREN); }
")"		{ return(tRPAREN); }
"{"		{ return(tLBLOCK); }
"}"		{ return(tRBLOCK); }
","		{ return(tCOMMA); }
"[]"		{ return(tARRAY); }
"*"		{ return(tSTAR); }
";"		{ /* parsing = 0; */
		  return(tSEMI); 
		}
"typedef"	{ return(tTYPEDEF); }
"struct"	{ return(tSTRUCT); }
{IDENT}		{ /* it's a identifier */
			  char *temp;
		  temp = (char *) malloc(strlen(yytext) + 1);
		  strcpy(temp, yytext);
		  yylval.cp = pool.findAndAdd(temp);
		  return(tIDENT);
		}
[ \n\t]		{ }
{INT}           {
		  yylval.i = atoi(yytext);
		  return(tINT);
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
