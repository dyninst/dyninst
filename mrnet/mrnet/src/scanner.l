HOSTNAME [a-zA-Z0-9][[a-zA-Z0-9\-".""_"]*
PORT     [0-9]+
COLON    ":"
SEMI     ";"
ARROW    "=>"

%{
#include <string.h>
#include "mrnet/src/NetworkGraph.h"

namespace MRN
{
#include "parser.tab.h"
int linenum=1;
}
%}
%option noyywrap
%%

{PORT}  { MRN::mrnlval.port = (unsigned short)atoi(yytext); return(PORT); }
{HOSTNAME} { MRN::mrnlval.hostname = strdup(yytext); return(HOSTNAME); }
{SEMI}  {return(SEMI); }
{ARROW} { return(ARROW); }
{COLON} { return(COLON); }
[ \t]+                  /* ignore whitespace */;
\n      {MRN::linenum++;}
"#".*  /* # starts a comment line */
%%
