%{
    #include "parser.tab.h"
    #include "headers/parser.h"
%}

/* float exponent */
EXP ([Ee][-+]?[0-9]+)

%%
"+" |
"-" |
"*" |
"/" |
":" |
"." |
"=" { return yytext[0]; }

"{" { return O_BRACKET; }
"}" { return C_BRACKET; }

 /* comparison ops */
">"     { yylval.fn = 1; return CMP; }
"<"     { yylval.fn = 2; return CMP; }
"<>"    { yylval.fn = 3; return CMP; }
"=="    { yylval.fn = 4; return CMP; }
">="    { yylval.fn = 5; return CMP; }
"<="    { yylval.fn = 6; return CMP; }

 /* keywords */
"define" { return DEFINE; }
"read" { return READ; }
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
\\\n    printf("c> "); /* ignore line continuation */
"\n"    { return EOL; }

.	{ yyerror("Mystery character %c\n", *yytext); }
%%