%{
    #include "parser.tab.h"
    #include "headers/parser.h"
    FILE * yyin;
%}
%option nounput
%option noinput
%option caseless

%x READ 
%x COMMENTO

%%
"read" BEGIN(READ);
<READ>[ \t]* 
<READ>[^ \t\n]+ {   /* Lettura file multipli */
                    // remove '\r' that cause errors
                    char * last = yytext + strlen(yytext) - 1;
                    if((*last) == '\r')
                        (*last) = '\0';  

                    yyin = fopen(yytext, "r");

                    if (!yyin)
                    {
                        yyerror("File inesistente\n");
                        yyin=stdin;
                    }           

                    yypush_buffer_state(yy_create_buffer(yyin, YY_BUF_SIZE));
                    
                    BEGIN(INITIAL);
                }
        <<EOF>> {
                    yypop_buffer_state();
                    if ( !YY_CURRENT_BUFFER )
                    {
                        yyterminate();
                    }
                }
"+" |
"-" |
"*" |
"/" |
":" |
"." |
"=" |
"(" |
")" |
"," |
"^" |
"_" |
"%" |
"\\" { return yytext[0]; }


"/*"            { BEGIN(COMMENTO); } 
<COMMENTO>"*/" { BEGIN(INITIAL); }
<COMMENTO>.    { }

"//"            { BEGIN(COMMENTO); } 
<COMMENTO>"\n" { BEGIN(INITIAL); }


"\""[^"]+"\"" {
                yylval.string = malloc(sizeof(char) * strlen(yytext));
                strcpy(yylval.string, yytext);
                return STRING;
            }

"{" { return O_BRACKET; }
"}" { return C_BRACKET; }
"[" { return O_ARRAY; }
"]" { return C_ARRAY; }

 /* comparison ops */
">"     { yylval.fn = 1; return CMP; }
"<"     { yylval.fn = 2; return CMP; }
"<>"    { yylval.fn = 3; return CMP; }
"=="    { yylval.fn = 4; return CMP; }
">="    { yylval.fn = 5; return CMP; }
"<="    { yylval.fn = 6; return CMP; }

 /* keywords   "do"    { return DO; } */

"define" { return DEFINE; }
"loop" { return LOOP; }
"from" { return FROM; }
"to" { return TO; }
"if" { return IF; }
"then"  { return THEN; }
"else"  { return ELSE; }
"fade" { return FADE; }
"delay" { return DELAY; }
"in" { return IN; }
"seconds" { return SECONDS; }
"sleep" { return SLEEP; }
"macro" { return MACRO; }
"print" { return PRINT; }
"input" { return INPUT; }
"delete" { return DELETE; }
"fixtures" { return FIXTURES; }
"extends" { return EXTENDS; }
"clear"  { return CLEAR; }
"setcolor" { return SETCOLOR; }
"resetcolor" { return RESETCOLOR; }
"connect" { return CONNECT; }
"disconnect" { return DISCONNECT; }

"//"[a-zA-Z][a-zA-Z0-9]*   { printf("commento\n"); }

&[a-zA-Z][a-zA-Z0-9]&   { printf("commento\n"); }

 /* names */
[a-zA-Z][a-zA-Z0-9]*  { 
                        yylval.string = malloc(sizeof(char) * strlen(yytext));
                        strcpy(yylval.string, yytext);
                        return NAME;
                    }

[0-9]+"."[0-9]* |
"."?[0-9]+ { yylval.d = atof(yytext); return NUMBER; }

"//".*  
[ \t]   { /* ignore white space */ }
[ \r]   { /* ignore carrige return */ }
\\\n    printf("c> "); /* ignore line continuation */
"\n"    { return EOL; }

.	{ yyerror(" Carattere non riconosciuto %c\n", *yytext); }
%%