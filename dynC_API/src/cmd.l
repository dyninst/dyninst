%option noyywrap

%x incl
%x comment
%x stringmode
%x cparse
%{

#include <string.h>
#include "BPatch_snippet.h"
#include "command.tab.h"

extern "C" {
 void set_lex_input(char *);
}

extern void commanderror (char const *s) {
    fprintf (stderr, "%s\n", s);
}


static char *input_str = NULL;
/*
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
*/
std::string string_buf = "";

int line_num = 1;

%}

%%

^"#"[ \t]*include[ \t]*\[\"<] { BEGIN incl; }

<incl>[^ \t\n\">]+  { commandlval.sval = strdup(yytext);  
                      BEGIN INITIAL; 
                      return INCLUDE;}

<INITIAL,cparse>"$"[A-Z]+        { return MACRO; }

<comment>{

[^*\n]*        /* eat anything that's not a '*' */
"*"+[^*/\n]*   /* eat up '*'s not followed by '/'s */
\n             ++line_num;
"*"+"/"        BEGIN(INITIAL);

}

load           { return LOAD; }

vars           { return VARS; }



int            { return INT; }

void           { return VOID; }

long           { return LONG; }

bool           { return BOOL; }

char           { return CHAR; }

double         { return DOUBLE; }

[0-9]+     { commandlval.ival = atoi(yytext); return NUMBER; }

entry|exit|preCall|subroutine|all     { commandlval.sval = strdup(yytext); return POINTTYPE; }

put            { return PUT; }

at             { return AT; }

in             { return IN; }

if             { return IF; }

with           { return WITH; }

termination    { return TERMINATION; }

replace        { return REPLACE; }

remove         { return REMOVE; }

call           { return F_CALL; }

calls          { return F_CALLS; }

trace          { return TRACE; }

untrace        { return UNTRACE; }

enable         { return ENABLE; }

disable        { return DISABLE; }

toggle         { return TOGGLE; }

snippets       { return SNIPPETS; }

mutations      { return MUTATIONS; }

detach         { return DETACH; }

reattach       { return REATTACH; }

print          { return PRINT; }

about          { return ABOUT; }

assert         { return ASSERT; }

kill           { return KILL; }

break          { return BREAK; }

write          { return WRITE; }

true           { return TRUE; }

false          { return FALSE; }

[A-za-z][A-Za-z0-9_]*	{ commandlval.sval = strdup(yytext); return IDENTIFIER; }

<*>"//".*      { /* inline comment */; ++line_num; }

<*>"/*"        BEGIN(comment);

[ \t]+         { /* whitespace */; }

\n             { ++line_num; }

\"             string_buf = ""; BEGIN(stringmode);

<stringmode>{
     
   \"           { /* saw closing quote - all done */
                  // this string handling code was taken from the flex manual
                  BEGIN(INITIAL);
                  char * cstr = new char[string_buf.size() + 1];
                  strcpy (cstr, string_buf.c_str());
                  commandlval.sval = strdup(cstr);
                  delete[] cstr;
                  return STRING;
                }

   \n           { /* error - unterminated string constant */
                  commandlval.context = "unterminated string constant"; 
                  commandlval.line_number = line_num; 
                  return ERROR;
                }

   \\[0-7]{1,3} { /* octal escape sequence */
                  int result;

                  (void) sscanf( yytext + 1, "%o", &result );

                  if ( result > 0xff ){
                      /* error, constant is out-of-bounds */
                      commandlval.context = "constant out of bounds"; 
                      commandlval.line_number = line_num; 
                      return ERROR;
                  }

                  string_buf += result;
                }

   \\[0-9]+     { /* generate error - bad escape sequence */
                  commandlval.context = "bad escape sequence";
                  commandlval.line_number = line_num;
                  return ERROR;
                }

   \\n          string_buf += '\n';
   \\t          string_buf += '\t';
   \\r          string_buf += '\r';
   \\b          string_buf += '\b';
   \\f          string_buf += '\f';

   \\(.|\n)     string_buf += yytext[1];
     
   [^\\\n\"]+   {
                  char *yptr = yytext;
                  while ( *yptr ) { string_buf += *yptr++; }
                }

}

[()]           { return yytext[0]; }

";"            { return SEMI; }

":"            { return COLON; }

"="            { return ASSIGN; }

"%{"           { return START_C_BLOCK; }

"%}"           { return END_C_BLOCK; }

\{             { return START_BLOCK; }

\}             { return END_BLOCK; }

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
