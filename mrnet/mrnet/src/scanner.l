HOSTNAME [a-zA-Z][[a-zA-Z0-9\-".""_"]*
PORT     [0-9]+
COLON    ":"
SEMI     ";"
ARROW    "=>"

%{
#include <string.h>
#include "mrnet/src/MC_NetworkGraph.h"
#include "mrnet/src/parser.tab.h"
int linenum=1;
%}
%option noyywrap
%%

{PORT}  { yylval.port = (unsigned short)atoi(yytext); return(PORT); }
{HOSTNAME} { yylval.hostname = strdup(yytext); return(HOSTNAME); }
{SEMI}  {return(SEMI); }
{ARROW} { return(ARROW); }
{COLON} { return(COLON); }
[ \t]+                  /* ignore whitespace */;
\n      {linenum++;}
"#".*  /* # starts a comment line */
%%
