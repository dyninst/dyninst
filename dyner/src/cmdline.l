%{
#if defined(i386_unknown_nt4_0)
#define YY_SKIP_YYWRAP
#define YY_NEVER_INTERACTIVE 1

int yywrap()
{
    return 1;
}
#endif

#include <string.h>
#include "BPatch_snippet.h"
#include "dyner.tab.h"

extern "C" {
void set_lex_input(char *);
}

static char *input_str = NULL;

#ifdef FLEX_SCANNER
#undef YY_INPUT

#define YY_INPUT(b,r,ms) (r = my_yyinput(b, ms))

int my_yyinput(char *buf, int max_size);
#else
#undef input
#undef unput

#define input()  (*input_str++)
#define unput(c) (*--input_str = c)

#endif


static int yyinput(void);

static int doubleQuote()
{
    int count;
    char ch;
    bool backSlash = false;

    count = 0;
    while (1) {
        ch = yyinput();
        yyleng++;
        switch (ch) {
            case '"':
                if (backSlash) {
                    /* \" */
                    yytext[count++] = '"';
                } else {
                    yyleng--;
                    yytext[count] = '\0';
		    dynerlval.sval = strdup(yytext);
                    return(STRING);
                }
                break;

            case '\n':
                /* New line before end of string */
                return(_ERROR);
                break;

	    case '\\':
                yytext[count++] = ch;
		backSlash = true;
		break;

	    case 'n':
		if (backSlash) {
		    yytext[count-1] = '\n';
		} else {
		    yytext[count++] = ch;
		}
		backSlash = false;
		break;

            default:
                yytext[count++] = ch;
		backSlash = false;
                break;
        }
    }
}

%}

%%

if			{ return IF; }

else			{ return ELSE; }

[0-9]+			{ dynerlval.ival = atoi(yytext); return NUMBER; }

[A-za-z][A-Za-z0-9_]*	{ dynerlval.sval = strdup(yytext); return IDENTIFIER; }

\<=			{ return LESS_EQ; }

\>=			{ return GREATER_EQ; }

==			{ return EQ; }

,			{ return COMMA; }

=			{ return ASSIGN; }

!=			{ return NOT_EQ; }
 
\{			{ return START_BLOCK; }

\}			{ return END_BLOCK; }

\$			{ return DOLLAR; }

\.			{ return DOT; }

;			{ return SEMI; }

[()<>+*/-]		{ return yytext[0]; }

&&			{ return AND; }

\|\|			{ return OR; }

[ \t\n]+		{ /* Nothing */; }

\"                      { int ret;
			  ret = doubleQuote();
			  return(ret);
			}

"++"			{ return PLUSPLUS; }

"--"			{ return MINUSMINUS; }

.			{ return _ERROR; }

%%

void set_lex_input(char *str)
{
    static int firstCall = 1;

    input_str = str;

    if (!firstCall) yy_flush_buffer(YY_CURRENT_BUFFER);
    firstCall = 0;
}

#ifdef FLEX_SCANNER
int my_yyinput(char *buf, int max_size)
{
    int sz = 0;

    while (*input_str != '\0' && sz < max_size) {
	*buf++ = *input_str++;
	sz++;
    }

    return sz;
}
#endif

#if !defined(i386_unknown_nt4_0)
int yywrap()
{
    return 1;
}
#endif
