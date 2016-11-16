%option noyywrap
%option nounput
%x comment
%x stringmode
%option yylineno
%option outfile="lex.dynC.C"
%option prefix="dynC"

D[0-9]
L[a-zA-Z_]
H[a-fA-F0-9]
E[Ee][+-]?{D}+
FS(f|F|l|L)
IS(u|U|l|L)*

%{

#include <stdio.h>
#include <string>
#include "BPatch_snippet.h"
#include <vector>
#include "dynC.tab.h"
#include "snippetGen.h"

int line_num = 1;

extern "C" {
  void set_lex_input(char *);
}

std::string lineStr;
static char *input_str = NULL;  


int yycolumn = 0;

extern bool fatalError;



#define YY_USER_ACTION dynClloc.first_line = dynClloc.last_line = line_num;\
        dynClloc.first_column = yycolumn; dynClloc.last_column = yycolumn+yyleng-1;\
        yycolumn += yyleng;\
        lineStr = yytext;

#define input() (*input_str++)
//#define yyinput() (*input_str++)
#define yylval dynClval

std::string c_string_buf = "";

const bool lexVerbose = false; // set to true for debug mode

//"$"{L}({L}|{D})* { yylval.sval = &yytext[1]; return(DYNINST_CALL); }

%}

%%

<comment>{                                                                      
                                                                                
[^*\n]*        /* eat anything that's not a '*' */ 
"*"+[^*/\n]*   /* eat up '*'s not followed by '/'s */                           
\n             ++line_num;                                                      
"*"+"/"        BEGIN(INITIAL);       
<<EOF>>        {yylval.context = "Syntax Error: Unterminated block comment"; return(D_ERROR);}
}   

"//".*      { if(lexVerbose)printf("Inline Comment\n") /* inline comment */;}                            
"/*"        { BEGIN(comment); }                                                  

char|int|long|float|double|short|void|bool { yylval.sval = strdup(yytext); return TYPE;}
"char"[ \t\v\f]*"*" {yylval.sval = strdup("char *"); return TYPE; }

"struct" {return STRUCT; }
"union" {return UNION; }
"enum" {return ENUM; }
 
"case" { return(CASE); }
"const" { return(D_CONST); }
"default" { return(DEFAULT); }
"else" { return(ELSE); }
"if" { return(IF); }
"return" { return(RETURN); }
"sizeof" { return(SIZEOF); }
"static" { return(STATIC); }
"switch" { return(SWITCH); }
"typedef" { return(TYPEDEF); }
"true" { return(D_TRUE); }
"false" { return(D_FALSE); }

"local" { return(LOCAL); }
"param" { return(PARAM); }
"global" { return(GLOBAL); }
"dyninst" { return(DYNINST); }
"inst" { return(INST); }
"func" { return(FUNC); }
"function" { return(FUNC); }
"reg" { return(REGISTER); }
"NULL" {return (NILL); }

[A-Za-z][A-Za-z0-9_]* {yylval.sval = strdup(yytext); if(lexVerbose) printf("ID: %s\n", yytext); return(IDENTIFIER); }



"-"?[0-9]+ { yylval.ival = atoi(yytext); return NUMBER;}

0[xX]{H}+{IS}? { return(CONSTANT); }
0{D}+{IS}? { return(CONSTANT); }
{D}+{IS}? { return(CONSTANT); }
L?'(\\.|[^\\'])+' { return(CONSTANT); }

{D}+{E}{FS}? { return(CONSTANT); }
{D}*"."{D}+({E})?{FS}? { return(CONSTANT); }
{D}+"."{D}*({E})?{FS}? { return(CONSTANT); }

\" {if(lexVerbose)printf("String Mode!\n"); c_string_buf = ""; BEGIN(stringmode);} 

<stringmode>{

   \"           { /* saw closing quote - all done */
                  // this string handling code was taken from the flex manual
                  if(lexVerbose)printf("string done\n");
                  BEGIN(INITIAL);                                                    
                  yylval.sval = strdup(c_string_buf.c_str());
                  return STRING;                                                     
                }

   \n           { /* error - unterminated strin constant */
                  yylval.context = "Unterminated string constant";
                  yycolumn = 1;
                  //yylval.line_number = line_num;                                                     line_num++;
                  return D_ERROR;
                }

   \\[0-7]{1,3} { /* octal escape sequence */
                  int result;                                                        
                  (void) sscanf( yytext + 1, "%o", &result );                        
                  if ( result > 0xff ){
                      /* error, constant is out-of-bounds */
                      yylval.context = "constant out of bounds";                
                     // yylval.line_number = line_num;                            
                      return D_ERROR;
                  }
                  c_string_buf += result;                                            
  
                }

   \\[0-9]+     { /* generate error - bad escape sequence */
                  yylval.context = "bad escape sequence";                       
                 // yylval.line_number = line_num;                                
                  return D_ERROR;
                }
   \\n          c_string_buf += '\n';                                                
   \\t          c_string_buf += '\t';                                                
   \\r          c_string_buf += '\r';                                                
   \\b          c_string_buf += '\b';                                             
   \\f          c_string_buf += '\f';                                                

   \\(.|\n)     c_string_buf += yytext[1];
   [^\\\n\"]+   {
                  char *yptr = yytext;
                  while ( *yptr ) { c_string_buf += *yptr++; }
                }
}
                                    

"..." { return(ELLIPSIS); }
">>=" { return(RIGHT_ASSIGN); }
"<<=" { return(LEFT_ASSIGN); }
"+=" { return(ADD_ASSIGN); }
"-=" { return(SUB_ASSIGN); }
"*=" { return(MUL_ASSIGN); }
"/=" { return(DIV_ASSIGN); }
"%=" { return(MOD_ASSIGN); }
"&=" { return(AND_ASSIGN); }
"^=" { return(XOR_ASSIGN); }
"|=" { return(OR_ASSIGN); }
">>" { return(RIGHT_OP); }
"<<" { return(LEFT_OP); }
"++" { return(INC_OP); }
"--" { return(DEC_OP); }
"->" { return(PTR_OP); }
"&&" { return(AND); }
"||" { return(OR); }
"<=" { return(LESS_EQ); }
">=" { return(GREATER_EQ); }
"==" { return(EQ); }
"!=" { return(NOT_EQ); }
";" { return(SEMI); }
("{"|"<%") { return('{'); }
("}"|"%>") { return('}'); }
"{%" { return(NOPEN); }
"%}" { return(NCLOSE); }
"," { return(COMMA); }
":" { return(COLON); }
"=" { return(ASSIGN); }
"(" { if(lexVerbose)printf("(\n"); return('('); }
")" { if(lexVerbose)printf(")\n"); return(')'); }
("[") { return('['); }
("]") { return(']'); }
"." { return(DOT); }
"&" { return('&'); }
"!" { return('!'); }
"~" { return('~'); }
"-" { return('-'); }
"+" { return('+'); }
"*" { return('*'); }
"/" { return('/'); }
"%" { return('%'); }
"<" { return('<'); }
">" { return('>'); }
"^" { return('^'); }
"|" { return('|'); }
"?" { return('?'); }
"`" { if(lexVerbose)printf("Backtick\n"); return(BACKTICK); }
[ \t\v\f] { }

^([^\n;{}])+\n {if(strstr(yytext, "//") != NULL){++line_num;}else{if(strncmp(yytext,"/*", 2) == 0){BEGIN(comment);++line_num;}else{if(lexVerbose)printf("No Semi!\n"); fatalError = true; dynClloc.first_column = yycolumn; yylval.context = "syntax error: missing ';'!"; yyless(yyleng - 1); return(D_ERROR);}}}

\n {if(lexVerbose)printf("New Line!\n"); line_num++; yycolumn = 0; lineStr = "";}

. { if(lexVerbose) printf("Unrecognized [%s]\n", yytext); /* ignore bad characters */ }



%%

void set_lex_input(char *str)
{
   static bool firstCall = true;
   YY_BUFFER_STATE bp;
   
   //db removed firstcall check
   if(!firstCall) dynC_flush_buffer(YY_CURRENT_BUFFER);
   yycolumn = 0;
   line_num = 1;
   
   input_str = str;
   bp = dynC_scan_string(str);
   (void)bp; // unused warning...
   firstCall = false;
}

