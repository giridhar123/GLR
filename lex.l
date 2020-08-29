%{
    #include "parser.tab.h"
    #include "progetto.h"
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

[\t] |
"    " { return TAB; }

 /* keywords */
"define" { return DEFINE; }
"read" { return READ; }

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