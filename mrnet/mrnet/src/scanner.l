%{
/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/
#include <string.h>
#include "mrnet/src/NetworkGraph.h"

namespace MRN
{
#include "parser.tab.h"


int curr = 0;
int len = 0;
int lineNum = 1;
#define MRN_MAX 1024
char line[MRN_MAX];

// for use when parsing MRNet configuration out of a buffer in memory
const char* mrnBufPtr = NULL;
unsigned int mrnBufRemaining = 0;

#undef getc
inline
int
getScanChar( FILE* file )
{
    if( curr == len )
    {
        char* ret = fgets( line, MRN_MAX, file );
        if( ret == NULL )
        {
            return EOF;
        }
        lineNum++;
        len = strlen( line );
        curr = 0;
    }
    return line[curr++];
}


inline
int
mrnGetInput( char* buf, unsigned int max_size )
{
    int ret = 0;

    if( mrnBufPtr != NULL )
    {
        // we're parsing from a buffer in memory
        unsigned int nConsumed =
            (( mrnBufRemaining < max_size ) ? mrnBufRemaining : max_size );
        memcpy( buf, mrnBufPtr, nConsumed );
        mrnBufPtr += nConsumed;
        mrnBufRemaining -= nConsumed;
        ret = nConsumed;
    }
    else
    {
        // we're reading from a file
        assert( mrnin != NULL );
        int c = getScanChar( mrnin );
        ret = (c == EOF) ? YY_NULL : (buf[0] = c, 1);
    }
    return ret;
}

#ifdef FLEX_SCANNER
#undef YY_INPUT
#define YY_INPUT(buf,result,max_size)       \
{                                           \
    result = MRN::mrnGetInput( buf, max_size );  \
}
#endif // FLEX_SCANNER

} // namespace MRN

%}

DIGIT    [0-9]
NZDIGIT  [1-9]
HOSTNAME [a-zA-Z0-9][[a-zA-Z0-9\-".""_"]*
COLON    ":"
SEMI     ";"
ARROW    "=>"
STAR     "*"

%option noyywrap never-interactive nounput
%%



{DIGIT}|{NZDIGIT}{DIGIT}*   {
                                unsigned int val = strtoul( yytext, NULL, 10 );
                                MRN::mrnlval.uval = val;
                                return UINT;
                            }
{HOSTNAME} { MRN::mrnlval.hostname = strdup(yytext); return(HOSTNAME); }
{SEMI}  {return(SEMI); }
{ARROW} { return(ARROW); }
{COLON} { return(COLON); }
{STAR}  { return STAR; }
[ \t]+                  /* ignore whitespace */;
\n      {MRN::lineNum++;}
"#".*  /* # starts a comment line */

%%
