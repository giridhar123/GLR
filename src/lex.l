%{
    #include "parser.tab.h"
    #include "headers/parser.h"
    FILE * yyin;
%}

%x READ 
%x COMMENTO
/* float exponent */
EXP ([Ee][-+]?[0-9]+)

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
                        exit(1);
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
"^" { return yytext[0]; }


"/*"            { BEGIN(COMMENTO); } 
<COMMENTO>"*/" { BEGIN(INITIAL); }
<COMMENTO>.    { }

"//"            { BEGIN(COMMENTO); } 
<COMMENTO>"\n" { BEGIN(INITIAL); }
<COMMENTO>.    { }


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

 /* keywords */
"define" { return DEFINE; }
"loop" { return LOOP; }
"from" { return FROM; }
"to" { return TO; }
"if" { return IF; }
"then"  { return THEN; }
"else"  { return ELSE; }
"do"    { return DO; }
"fade" { return FADE; }
"delay" { return DELAY; }
"in" { return IN; }
"seconds" { return SECONDS; }
"sleep" { return SLEEP; }
"macro" { return MACRO; }
"print" { return PRINT; }
"input" { return INPUT; }

"//"[a-zA-Z][a-zA-Z0-9]*   { printf("commento\n"); }

&[a-zA-Z][a-zA-Z0-9]&   { printf("commento\n"); }

 /* names */
[a-zA-Z][a-zA-Z0-9]*  { 
                        yylval.string = malloc(sizeof(char) * strlen(yytext));
                        strcpy(yylval.string, yytext);
                        return NAME;
                    }

[0-9]+"."[0-9]*{EXP}? |
"."?[0-9]+{EXP}? { yylval.d = atof(yytext); return NUMBER; }

"//".*  
[ \t]   /* ignore white space */ 
[ \r]   /* ignore carrige return */
\\\n    printf("c> "); /* ignore line continuation */
"\n"    { return EOL; }

.	{ yyerror("Mystery character %c\n", *yytext); }
%%