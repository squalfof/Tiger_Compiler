
%{
#include <string.h>
#include "util.h"
#include "lex_tokens.h"
#include "errormsg.h"

int charPos=1;
char buf_array[256];

int yywrap(void)
{
 charPos=1;
 return 1;
}


void adjust(void)
{
 EM_tokPos=charPos;
 charPos+=yyleng;
}

%}

%x COMMENT
%x STR

%%


"/*"                  { adjust(); 
						BEGIN(COMMENT); }
<COMMENT>\n           /* eat up newline */
<COMMENT>[^*\n]*      /* eat up any char except * */
<COMMENT>"*"+[^*/\n]* /* eat separate * without */
<COMMENT>"*"+"/"      { adjust(); 
			BEGIN(INITIAL); }
						
						
"\""              { 	adjust(); 
			BEGIN(STR); }
<STR>[^"]*        { 	yymore(); /* append the end quote */ }
<STR>"\""         { 	adjust();
			yylval.sval = String((char*)yytext);
			/* yylval.sval[yyleng - 1] = '\0'; */
			BEGIN(INITIAL); 
			return STRING; }

[ \t]*	{ adjust(); continue; }
\n	{ adjust(); EM_newline(); continue;}
","	{ adjust(); return COMMA;}
":"	{ adjust(); return COLON; }
";"    	{ adjust(); return SEMICOLON; }
"("    	{ adjust(); return LPAREN; }
")"    	{ adjust(); return RPAREN; }
"["    	{ adjust(); return LBRACK; }
"]"    	{ adjust(); return RBRACK; }
"{"    	{ adjust(); return LBRACE; }
"}"    	{ adjust(); return RBRACE; }
"."    	{ adjust(); return DOT; }
"+"    	{ adjust(); return PLUS; }
"-"    	{ adjust(); return MINUS; }
"*"    	{ adjust(); return TIMES; }
"/"    	{ adjust(); return DIVIDE; }
"="    	{ adjust(); return EQ; }
"<>"   	{ adjust(); return NEQ; }
"<"    	{ adjust(); return LT; }
"<="   	{ adjust(); return LE; }
">"    	{ adjust(); return GT; }
">="   	{ adjust(); return GE; }
"&"    	{ adjust(); return AND; }
"|"    	{ adjust(); return OR; }
":="   	{ adjust(); return ASSIGN; }


array    { adjust(); return ARRAY; }
if       { adjust(); return IF; }
then     { adjust(); return THEN; }
else     { adjust(); return ELSE; }
while    { adjust(); return WHILE; }
for      { adjust(); return FOR; }
to       { adjust(); return TO; }
do       { adjust(); return DO; }
let      { adjust(); return LET; }
in       { adjust(); return IN; }
end      { adjust(); return END; }
of       { adjust(); return OF; }
break    { adjust(); return BREAK; }
nil      { adjust(); return NIL; }
function { adjust(); return FUNCTION; }
var      { adjust(); return VAR; }
type     { adjust(); return TYPE; }


[0-9]+			{	adjust(); 
				yylval.ival=atoi(yytext); 
				return INT; 
			}
[a-zA-Z]+[a-zA-Z_0-9]*  {	adjust(); 
				yylval.sval = String((char*)yytext); 
				return ID; 
			}


.	 		{	adjust(); EM_error(EM_tokPos,"illegal token");}

<<EOF>>          	{ 	yyterminate(); }
